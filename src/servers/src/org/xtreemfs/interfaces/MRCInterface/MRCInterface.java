package org.xtreemfs.interfaces.MRCInterface;

import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.Exceptions.*;




public class MRCInterface
{
    public static final int DEFAULT_ONCRPC_PORT = 32636;
    public static final int DEFAULT_ONCRPCS_PORT = 32636;
    public static final int DEFAULT_HTTP_PORT = 30636;


    public static int getVersion() { return 1200; }

    public static Request createRequest( ONCRPCRequestHeader header ) throws Exception
    {
        switch( header.getProcedure() )
        {
            case 1201: return new accessRequest();
            case 1202: return new chmodRequest();
            case 1203: return new chownRequest();
            case 1230: return new ftruncateRequest();
            case 1205: return new getattrRequest();
            case 1206: return new getxattrRequest();
            case 1207: return new linkRequest();
            case 1208: return new listxattrRequest();
            case 1209: return new mkdirRequest();
            case 1211: return new openRequest();
            case 1212: return new readdirRequest();
            case 1213: return new removexattrRequest();
            case 1214: return new renameRequest();
            case 1215: return new rmdirRequest();
            case 1217: return new setattrRequest();
            case 1218: return new setxattrRequest();
            case 1219: return new statvfsRequest();
            case 1220: return new symlinkRequest();
            case 1221: return new unlinkRequest();
            case 1222: return new utimensRequest();
            case 1251: return new xtreemfs_checkpointRequest();
            case 1223: return new xtreemfs_check_file_existsRequest();
            case 1252: return new xtreemfs_dump_databaseRequest();
            case 1224: return new xtreemfs_get_suitable_osdsRequest();
            case 1231: return new xtreemfs_lsvolRequest();
            case 1233: return new xtreemfs_listdirRequest();
            case 1210: return new xtreemfs_mkvolRequest();
            case 1225: return new xtreemfs_renew_capabilityRequest();
            case 1226: return new xtreemfs_replica_addRequest();
            case 1232: return new xtreemfs_replica_listRequest();
            case 1227: return new xtreemfs_replica_removeRequest();
            case 1253: return new xtreemfs_restore_databaseRequest();
            case 1228: return new xtreemfs_restore_fileRequest();
            case 1216: return new xtreemfs_rmvolRequest();
            case 1250: return new xtreemfs_shutdownRequest();
            case 1229: return new xtreemfs_update_file_sizeRequest();

            default: throw new Exception( "unknown request tag " + Integer.toString( header.getProcedure() ) );
        }
    }
            
    public static Response createResponse( ONCRPCResponseHeader header ) throws Exception
    {
        switch( header.getXID() )
        {
            case 1201: return new accessResponse();            case 1202: return new chmodResponse();            case 1203: return new chownResponse();            case 1230: return new ftruncateResponse();            case 1205: return new getattrResponse();            case 1206: return new getxattrResponse();            case 1207: return new linkResponse();            case 1208: return new listxattrResponse();            case 1209: return new mkdirResponse();            case 1211: return new openResponse();            case 1212: return new readdirResponse();            case 1213: return new removexattrResponse();            case 1214: return new renameResponse();            case 1215: return new rmdirResponse();            case 1217: return new setattrResponse();            case 1218: return new setxattrResponse();            case 1219: return new statvfsResponse();            case 1220: return new symlinkResponse();            case 1221: return new unlinkResponse();            case 1222: return new utimensResponse();            case 1251: return new xtreemfs_checkpointResponse();            case 1223: return new xtreemfs_check_file_existsResponse();            case 1252: return new xtreemfs_dump_databaseResponse();            case 1224: return new xtreemfs_get_suitable_osdsResponse();            case 1231: return new xtreemfs_lsvolResponse();            case 1233: return new xtreemfs_listdirResponse();            case 1210: return new xtreemfs_mkvolResponse();            case 1225: return new xtreemfs_renew_capabilityResponse();            case 1226: return new xtreemfs_replica_addResponse();            case 1232: return new xtreemfs_replica_listResponse();            case 1227: return new xtreemfs_replica_removeResponse();            case 1253: return new xtreemfs_restore_databaseResponse();            case 1228: return new xtreemfs_restore_fileResponse();            case 1216: return new xtreemfs_rmvolResponse();            case 1250: return new xtreemfs_shutdownResponse();            case 1229: return new xtreemfs_update_file_sizeResponse();
            default: throw new Exception( "unknown response XID " + Integer.toString( header.getXID() ) );
        }
    }    

    public static ONCRPCException createException( String exception_type_name ) throws java.io.IOException
    {
        if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::ConcurrentModificationException") ) return new ConcurrentModificationException();
        else if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::errnoException") ) return new errnoException();
        else if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::InvalidArgumentException") ) return new InvalidArgumentException();
        else if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::MRCException") ) return new MRCException();
        else if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::ProtocolException") ) return new ProtocolException();
        else if ( exception_type_name.equals("org::xtreemfs::interfaces::MRCInterface::RedirectException") ) return new RedirectException();
        else throw new java.io.IOException( "unknown exception type " + exception_type_name );
    }

}
