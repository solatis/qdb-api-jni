package net.quasardb.qdb.jni;

public final class qdb_timespec {
  public long tv_sec;
  public long tv_nsec;

  public qdb_timespec(long tv_sec, long tv_nsec){
    this.tv_sec = tv_sec;
    this.tv_nsec = tv_nsec;
  }
}
