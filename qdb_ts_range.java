package net.quasardb.qdb.jni;

public final class qdb_ts_range {
  qdb_timespec begin;
  qdb_timespec end;

  public qdb_ts_range(qdb_timespec begin, qdb_timespec end){
    this.begin = begin;
    this.end = end;
  }
}
