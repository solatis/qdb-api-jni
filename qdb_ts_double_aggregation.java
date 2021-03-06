package net.quasardb.qdb.jni;

public final class qdb_ts_double_aggregation {
  protected long aggregation_type;
  protected qdb_ts_filtered_range filtered_range;

  protected long count;
  protected qdb_ts_double_point result;

  public qdb_ts_double_aggregation(qdb_ts_filtered_range filtered_range, long aggregation_type) {
    this.filtered_range = filtered_range;
    this.aggregation_type = aggregation_type;
    this.count = -1;
  }

  public qdb_ts_double_aggregation(qdb_ts_filtered_range filtered_range, long aggregation_type, long count, qdb_ts_double_point result) {
    this.filtered_range = filtered_range;
    this.aggregation_type = aggregation_type;
    this.count = count;
    this.result = result;
  }

  public qdb_ts_filtered_range getFilteredRange() {
    return this.filtered_range;
  }

  public long getAggregationType() {
    return this.aggregation_type;
  }

  public long getCount() {
    return this.count;
  }

  public qdb_ts_double_point getResult() {
    return this.result;
  }
}
