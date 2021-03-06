/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.xtreemfs.common.libxtreemfs.jni.generated;

public class ServiceAddresses {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected ServiceAddresses(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(ServiceAddresses obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        xtreemfs_jniJNI.delete_ServiceAddresses(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public ServiceAddresses(String address) {
    this(xtreemfs_jniJNI.new_ServiceAddresses__SWIG_0(address), true);
  }

  public ServiceAddresses(StringVector addresses) {
    this(xtreemfs_jniJNI.new_ServiceAddresses__SWIG_1(StringVector.getCPtr(addresses), addresses), true);
  }

}
