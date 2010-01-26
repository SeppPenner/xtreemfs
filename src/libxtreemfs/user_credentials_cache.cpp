// Copyright 2009-2010 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "xtreemfs/user_credentials_cache.h"
using namespace xtreemfs;


UserCredentialsCache::~UserCredentialsCache()
{
  for 
  ( 
    std::vector<YIELD::platform::SharedLibrary*>::iterator 
      policy_shared_library_i = policy_shared_libraries.begin(); 
    policy_shared_library_i != policy_shared_libraries.end(); 
    policy_shared_library_i++ 
  )
    YIELD::platform::SharedLibrary::decRef( **policy_shared_library_i );

#ifndef _WIN32
  for 
  ( 
    std::map<std::string,std::map<std::string,std::pair<uid_t,gid_t>*>*>::iterator 
      i = user_credentials_to_passwd_cache.begin(); 
    i != user_credentials_to_passwd_cache.end(); 
    i++ 
  )
  {
    for 
    ( 
      std::map<std::string,std::pair<uid_t,gid_t>*>::iterator 
        j = i->second->begin(); 
      j != i->second->end();
      j++ 
    )
      delete j->second;

    delete i->second;
  }

  for 
  ( 
    std::map<gid_t,std::map<uid_t,UserCredentials*>*>::iterator 
      i = passwd_to_user_credentials_cache.begin(); 
    i != passwd_to_user_credentials_cache.end(); 
    i++ 
  )
  {
    for 
    ( 
      std::map<uid_t,UserCredentials*>::iterator 
        j = i->second->begin(); 
      j != i->second->end(); 
      j++ 
    )
      delete j->second;

    delete i->second;
  }
#endif
}

void* UserCredentialsCache::getPolicyFunction( const char* name )
{
  for 
  ( 
    std::vector<YIELD::platform::SharedLibrary*>::iterator 
      policy_shared_library_i = policy_shared_libraries.begin(); 
    policy_shared_library_i != policy_shared_libraries.end(); 
    policy_shared_library_i++ 
  )
  {
    void* policy_function = ( *policy_shared_library_i )->getFunction( name );
    if ( policy_function != NULL )
      return policy_function;
  }

  std::vector<YIELD::platform::Path> policy_dir_paths;
  policy_dir_paths.push_back( YIELD::platform::Path() );
#ifdef _WIN32
  policy_dir_paths.push_back( "src\\policies\\lib" );
#else
  policy_dir_paths.push_back( "src/policies/lib" );
  policy_dir_paths.push_back( "/lib/xtreemfs/policies/" );
#endif

  YIELD::platform::auto_Volume volume = new YIELD::platform::Volume;
  for 
  ( 
    std::vector<YIELD::platform::Path>::iterator 
      policy_dir_path_i = policy_dir_paths.begin(); 
    policy_dir_path_i != policy_dir_paths.end(); 
    policy_dir_path_i++ 
  )
  {
    std::vector<YIELD::platform::Path> file_names;
    volume->listdir( *policy_dir_path_i, file_names );

    for 
    ( 
      std::vector<YIELD::platform::Path>::iterator 
        file_name_i = file_names.begin(); 
      file_name_i != file_names.end(); 
      file_name_i++ 
    )
    {
      const std::string& 
        file_name = static_cast<const std::string&>( *file_name_i );

      std::string::size_type 
          dll_pos = file_name.find( SHLIBSUFFIX );

      if 
      ( 
        dll_pos != std::string::npos && 
        dll_pos != 0 && 
        file_name[dll_pos-1] == '.' 
      )
      {
        YIELD::platform::Path policy_shared_library_path 
          = *policy_dir_path_i  + file_name;

        YIELD::platform::auto_SharedLibrary policy_shared_library 
          = YIELD::platform::SharedLibrary::open( policy_shared_library_path );

        if ( policy_shared_library != NULL )
        {
          void* policy_function = policy_shared_library->getFunction( name );
          if ( policy_function != NULL )
            policy_shared_libraries.push_back( policy_shared_library.release() );
          return policy_function;
        }
      }
    }
  }

  return NULL;
}

#ifndef _WIN32
void UserCredentialsCache::getpasswdFromUserCredentials
( 
  const std::string& user_id, 
  const std::string& group_id, 
  uid_t& out_uid, 
  gid_t& out_gid 
)
{
//#ifdef _DEBUG
//  if 
//  ( 
//    ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//    log != NULL 
//  )
//    log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//      "xtreemfs::Proxy: getting passwd from UserCredentials (user_id=" << 
//      user_id << ", group_id=" << group_id << ").";
//#endif

  user_credentials_to_passwd_cache_lock.acquire();

  std::map<std::string,std::map<std::string,std::pair<uid_t, gid_t>*>*>::iterator 
    group_i = user_credentials_to_passwd_cache.find( group_id );

  if ( group_i != user_credentials_to_passwd_cache.end() )
  {
    std::map<std::string,std::pair<uid_t,gid_t>*>::iterator user_i = 
      group_i->second->find( user_id );

    if ( user_i != group_i->second->end() )
    {
      out_uid = user_i->second->first;
      out_gid = user_i->second->second;

      user_credentials_to_passwd_cache_lock.release();

//#ifdef _DEBUG
//      if 
//      ( 
//        ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//        log != NULL 
//      )
//        log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//          "xtreemfs::Proxy: found user and group IDs in cache, " << 
//          user_id << "=" << out_uid << ", " << group_id << "=" << 
//          out_gid << ".";
//#endif


      return;
    }
  }

  user_credentials_to_passwd_cache_lock.release();


  bool have_passwd = false;
  if ( get_passwd_from_user_credentials )
  {
//#ifdef _DEBUG
//    if 
//    ( 
//      ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//      log != NULL 
//    )
//      log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//        "xtreemfs::Proxy: calling get_passwd_from_user_credentials_ret " <<
//        "with user_id=" << user_id << ", group_id=" << group_id << ".";
//#endif

    int get_passwd_from_user_credentials_ret = 
      get_passwd_from_user_credentials
      ( 
        user_id.c_str(), 
        group_id.c_str(), 
        &out_uid, 
        &out_gid 
      );
    if ( get_passwd_from_user_credentials_ret >= 0 )
      have_passwd = true;
    //else if 
    //( 
    //  ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == 
    //  PROXY_FLAG_TRACE_AUTH && log != NULL 
    //)
    //  log->getStream( YIELD::platform::Log::LOG_ERR ) << 
    //    "xtreemfs::Proxy: get_passwd_from_user_credentials_ret with user_id=" 
    //    << user_id << ", group_id=" << group_id << " failed with errno=" << 
    //    ( get_passwd_from_user_credentials_ret * -1 );
  }

  if ( !have_passwd )
  {
//#ifdef _DEBUG
//    if 
//    ( 
//      ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//      log != NULL 
//    )
//      log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//        "xtreemfs::Proxy: calling getpwnam_r and getgrnam_r with user_id=" << 
//        user_id << ", group_id=" << group_id << ".";
//#endif

    struct passwd pwd, *pwd_res;
    int pwd_buf_len = sysconf( _SC_GETPW_R_SIZE_MAX );
    if ( pwd_buf_len <= 0 ) pwd_buf_len = 1024;
    char* pwd_buf = new char[pwd_buf_len];

    struct group grp, *grp_res;
    int grp_buf_len = sysconf( _SC_GETGR_R_SIZE_MAX );
    if ( grp_buf_len <= 0 ) grp_buf_len = 1024;
    char* grp_buf = new char[grp_buf_len];
 
    if 
    ( 
      getpwnam_r
      ( 
        user_id.c_str(), 
        &pwd, 
        pwd_buf, 
        pwd_buf_len, 
        &pwd_res 
      ) == 0 && pwd_res != NULL 
      &&
      getgrnam_r
      ( 
        group_id.c_str(), 
        &grp, 
        grp_buf, 
        grp_buf_len, 
        &grp_res 
      ) == 0 && grp_res != NULL 
    )
    {
      out_uid = pwd_res->pw_uid;
      out_gid = grp_res->gr_gid;
    }
    else
    {
      out_uid = 0;
      out_gid = 0;
      //if ( log != NULL )
      //  log->getStream( YIELD::platform::Log::LOG_WARNING ) << 
      //    "xtreemfs::Proxy: getpwnam_r and getgrnam_r with user_id=" << 
      //    user_id << ", group_id=" << group_id << " failed, errno=" << 
      //    errno << ", setting user/group to root.";
    }

    delete [] pwd_buf;
    delete [] grp_buf;
  }

//#ifdef _DEBUG
//  if 
//  ( 
//    ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//      log != NULL 
//  )
//    log->getStream( YIELD::platform::Log::LOG_DEBUG ) << "xtreemfs::Proxy: " <<
//      user_id << "=" << out_uid << ", " << 
//      group_id << "=" << out_gid << 
//      ", storing in cache.";
//#endif

  user_credentials_to_passwd_cache_lock.acquire();

  if ( group_i != user_credentials_to_passwd_cache.end() )
  {
    group_i->second->insert
    ( 
      std::make_pair
      ( 
        user_id, 
        new std::pair<uid_t,gid_t>( out_uid, out_gid )  
      ) 
    );
  }
  else
  {
    user_credentials_to_passwd_cache[group_id] = 
      new std::map<std::string,std::pair<uid_t,gid_t>*>;

    user_credentials_to_passwd_cache[group_id]->insert
    ( 
      std::make_pair( user_id, new std::pair<uid_t,gid_t>( out_uid, out_gid ) ) 
    );
  }

  user_credentials_to_passwd_cache_lock.release();
}

bool UserCredentialsCache::getUserCredentialsFrompasswd
( 
  uid_t uid, 
  gid_t gid, 
  UserCredentials& out_user_credentials 
)
{
//#ifdef _DEBUG
//  if 
//  ( 
//    ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH &&
//    log != NULL 
//  )
//    log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//      "xtreemfs::Proxy: getting UserCredentials from passwd (uid=" << 
//      uid << ", gid=" << gid << ").";
//#endif

  passwd_to_user_credentials_cache_lock.acquire();

  std::map<gid_t,std::map<uid_t,UserCredentials*>*>
    ::iterator group_i = passwd_to_user_credentials_cache.find( gid );

  if ( group_i != passwd_to_user_credentials_cache.end() )
  {
    std::map<uid_t,UserCredentials*>::iterator 
      user_i = group_i->second->find( uid );

    if ( user_i != group_i->second->end() )
    {
      out_user_credentials = *user_i->second;

      passwd_to_user_credentials_cache_lock.release();

//#ifdef _DEBUG
//      if 
//      ( 
//        ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//        log != NULL 
//      )
//        log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//          "xtreemfs::Proxy: found UserCredentials in cache, " << 
//          uid << "=" << out_user_credentials.get_user_id() << ", " << 
//          gid << "=" << out_user_credentials.get_group_ids()[0] << ".";
//#endif

      return true;
    }
  }

  passwd_to_user_credentials_cache_lock.release();


  if ( get_user_credentials_from_passwd )
  {
//#ifdef _DEBUG
//    if 
//    ( 
//      ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//      log != NULL 
//    )
//      log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//        "xtreemfs::Proxy: calling get_user_credentials_from_passwd with uid=" 
//        << uid << ", gid=" << gid << ".";
//#endif

    size_t user_id_len = 0, group_ids_len = 0;
    int get_user_credentials_from_passwd_ret 
      = get_user_credentials_from_passwd
        ( 
          uid, 
          gid, 
          NULL, 
          &user_id_len, 
          NULL, 
          &group_ids_len 
        );
    if ( get_user_credentials_from_passwd_ret >= 0 )
    {
//#ifdef _DEBUG
//      if 
//      ( 
//        ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == PROXY_FLAG_TRACE_AUTH && 
//        log != NULL 
//      )
//        log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//          "xtreemfs::Proxy: calling get_user_credentials_from_passwd " <<
//          "with uid=" << uid << ", gid=" << gid << " returned " << 
//          get_user_credentials_from_passwd_ret << 
//          ", allocating space for UserCredentials.";
//#endif

      if ( user_id_len > 0 ) // group_ids_len can be 0
      {
        char* user_id = new char[user_id_len];
        char* group_ids = group_ids_len > 0 ? new char[group_ids_len] : NULL;

        get_user_credentials_from_passwd_ret = 
          get_user_credentials_from_passwd
          ( 
            uid, 
            gid, 
            user_id, 
            &user_id_len, 
            group_ids, 
            &group_ids_len 
          );
        if ( get_user_credentials_from_passwd_ret >= 0 )
        {
          out_user_credentials.set_user_id( user_id );

          if ( group_ids_len > 0 )
          {
            char* group_ids_p = group_ids;
            StringSet group_ids_ss;
            while 
            ( 
              static_cast<size_t>( group_ids_p - group_ids ) < group_ids_len 
            )
            {
              group_ids_ss.push_back( group_ids_p );
              group_ids_p += group_ids_ss.back().size() + 1;
            }
          
            out_user_credentials.set_group_ids( group_ids_ss );
          }
          else
            out_user_credentials.set_group_ids
            ( 
              StringSet( "" ) 
            );

//#ifdef _DEBUG
//          if 
//          ( 
//            ( this->get_flags() & PROXY_FLAG_TRACE_AUTH ) == 
//            PROXY_FLAG_TRACE_AUTH &&
//            log != NULL 
//          )
//            log->getStream( YIELD::platform::Log::LOG_DEBUG ) << 
//              "xtreemfs::Proxy: get_user_credentials_from_passwd succeeded, " << 
//              uid << "=" << out_user_credentials.get_user_id() << ", " << 
//              gid << "=" << out_user_credentials.get_group_ids()[0] << ".";
//#endif

          // Drop down to insert the credentials into the cache
        }
        else
          return false;
      }
    }
    else
      return false;
  }
  else
  {
    if ( uid != -1 )
    {
      struct passwd pwd, *pwd_res;
      int pwd_buf_len = sysconf( _SC_GETPW_R_SIZE_MAX );
      if ( pwd_buf_len <= 0 ) pwd_buf_len = 1024;
      char* pwd_buf = new char[pwd_buf_len];

      if ( getpwuid_r( uid, &pwd, pwd_buf, pwd_buf_len, &pwd_res ) == 0 )
      {
        if ( pwd_res != NULL && pwd_res->pw_name != NULL )
        {
          out_user_credentials.set_user_id( pwd_res->pw_name );
          delete [] pwd_buf;
        } 
        else
        {
          delete [] pwd_buf;
          return false;
        }
      } 
      else
      {
        delete [] pwd_buf;
        return false;
      }
    } 
    else
      out_user_credentials.set_user_id( "" );

    if ( gid != -1 )
    {
      struct group grp, *grp_res;
      int grp_buf_len = sysconf( _SC_GETGR_R_SIZE_MAX );
      if ( grp_buf_len <= 0 ) grp_buf_len = 1024;
      char* grp_buf = new char[grp_buf_len];

      if ( getgrgid_r( gid, &grp, grp_buf, grp_buf_len, &grp_res ) == 0 )
      {
        if ( grp_res != NULL && grp_res->gr_name != NULL )
        {
          out_user_credentials.set_group_ids
          ( 
            StringSet( grp_res->gr_name ) 
          );
          delete [] grp_buf;
          // Drop down to insert the credentials into the cache
        }
        else
        {
          delete [] grp_buf;
          return false;
        }
      } 
      else
      {
        delete [] grp_buf;
        return false;
      }
    } 
    else
      out_user_credentials.set_group_ids
      ( 
        StringSet( "" ) 
      );
      // Drop down to insert the credentials into the cache
  }

  passwd_to_user_credentials_cache_lock.acquire();

  if ( group_i != passwd_to_user_credentials_cache.end() )
  {
    group_i->second->insert
    ( 
      std::make_pair
      ( 
        uid, 
        new UserCredentials( out_user_credentials ) 
      ) 
    );
  }
  else
  {
    passwd_to_user_credentials_cache[gid] = 
      new std::map<uid_t,UserCredentials*>;

    passwd_to_user_credentials_cache[gid]->insert
    ( 
      std::make_pair
      ( 
        uid, 
        new UserCredentials( out_user_credentials ) 
      ) 
    );
  }

  passwd_to_user_credentials_cache_lock.release();

  return true;
}
#endif
