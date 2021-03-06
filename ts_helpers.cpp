#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "ts_helpers.h"

void
timespecToNative(JNIEnv *env, jobject input, qdb_timespec_t * output) {
  // qdb_timespec -> tv_sec, tv_nsec
  jfieldID sec_field, nsec_field;
  jclass object_class;

  object_class = env->GetObjectClass(input);

  sec_field = env->GetFieldID(object_class, "tv_sec", "J");
  nsec_field = env->GetFieldID(object_class, "tv_nsec", "J");
  output->tv_sec = env->GetLongField(input, sec_field);
  output->tv_nsec = env->GetLongField(input, nsec_field);
}

void
nativeToTimespec(JNIEnv *env, qdb_timespec_t input, jobject * output) {
  jclass timespec_class = env->FindClass("net/quasardb/qdb/jni/qdb_timespec");
  jmethodID constructor = env->GetMethodID(timespec_class, "<init>", "(JJ)V");

  *output = env->NewObject(timespec_class,
                           constructor,
                           input.tv_sec,
                           input.tv_nsec);
}

void
rangeToNative(JNIEnv *env, jobject input, qdb_ts_range_t * native) {
  jfieldID beginField, endField;
  jclass objectClass;

  objectClass = env->GetObjectClass(input);

  beginField = env->GetFieldID(objectClass, "begin", "Lnet/quasardb/qdb/jni/qdb_timespec;");
  endField = env->GetFieldID(objectClass, "end", "Lnet/quasardb/qdb/jni/qdb_timespec;");

  timespecToNative(env, env->GetObjectField(input, beginField), &(native->begin));
  timespecToNative(env, env->GetObjectField(input, endField), &(native->end));
}

void
rangesToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_range_t * native) {
  qdb_ts_range_t * cur = native;
  for (size_t i = 0; i < count; ++i) {
    jobject point = (jobject)(env->GetObjectArrayElement(input, i));

    rangeToNative(env, point, cur++);
  }
}

void
filterToNative(JNIEnv *env, jobject input, qdb_ts_filter_t * native) {
  native->type = qdb_ts_filter_none;
}

void
nativeToFilter(JNIEnv *env, qdb_ts_filter_t input, jobject * output) {
  assert (input.type == qdb_ts_filter_none);

  jclass no_filter_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_no_filter");
  jmethodID constructor = env->GetMethodID(no_filter_class, "<init>", "()V");

  *output = env->NewObject(no_filter_class,
                           constructor);
}

void
filteredRangeToNative(JNIEnv *env, jobject input, qdb_ts_filtered_range_t * native) {
  jfieldID rangeField, filterField;
  jclass objectClass;

  objectClass = env->GetObjectClass(input);
  rangeField = env->GetFieldID(objectClass, "range", "Lnet/quasardb/qdb/jni/qdb_ts_range;");
  filterField = env->GetFieldID(objectClass, "filter", "Lnet/quasardb/qdb/jni/qdb_ts_filter;");

  rangeToNative(env, env->GetObjectField(input, rangeField), &(native->range));
  filterToNative(env, env->GetObjectField(input, filterField), &(native->filter));
}

void
filteredRangesToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_filtered_range_t * native) {
  qdb_ts_filtered_range_t * cur = native;

  for (size_t i = 0; i < count; ++i) {
    jobject point = (jobject)(env->GetObjectArrayElement(input, i));

    filteredRangeToNative(env, point, cur++);
  }
}

void
nativeToRange(JNIEnv * env, qdb_ts_range_t native, jobject * output) {
  jclass point_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_range");
  jmethodID constructor = env->GetMethodID(point_class, "<init>", "(Lnet/quasardb/qdb/jni/qdb_timespec;Lnet/quasardb/qdb/jni/qdb_timespec;)V");

  jobject begin;
  jobject end;

  nativeToTimespec(env, native.begin, &begin);
  nativeToTimespec(env, native.end, &end);

  *output = env->NewObject(point_class,
                           constructor,
                           begin,
                           end);
}

void
nativeToFilteredRange(JNIEnv * env, qdb_ts_filtered_range_t native, jobject * output) {
  jclass filtered_range_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_filtered_range");
  jmethodID constructor = env->GetMethodID(filtered_range_class, "<init>", "(Lnet/quasardb/qdb/jni/qdb_ts_range;Lnet/quasardb/qdb/jni/qdb_ts_filter;)V");

  jobject range, filter;

  nativeToRange(env, native.range, &range);
  nativeToFilter(env, native.filter, &filter);

  *output = env->NewObject(filtered_range_class,
                           constructor,
                           range,
                           filter);
}

void
columnsToNative(JNIEnv * env, jobjectArray columns, qdb_ts_column_info * native_columns, size_t column_count) {
  jfieldID nameField, typeField;
  jclass objectClass;
  for (size_t i = 0; i < column_count; ++i) {
    jobject object = (jobject) (env->GetObjectArrayElement(columns, i));

    objectClass = env->GetObjectClass(object);
    nameField = env->GetFieldID(objectClass, "name", "Ljava/lang/String;");
    typeField = env->GetFieldID(objectClass, "type", "I");
    jstring name = (jstring)env->GetObjectField(object, nameField);

    native_columns[i].type = (qdb_ts_column_type)(env->GetIntField(object, typeField));

    // Is there a better way to do this ? Because we're using strdup here, we need a separate
    // release function which is fragile.
    native_columns[i].name = strdup(StringUTFChars(env, name));
  }
}

void
releaseNative(qdb_ts_column_info * native_columns, size_t column_count) {
  for (size_t i = 0; i < column_count; ++i) {
    free((void *)(native_columns[i].name));
  }
}

void
nativeToColumns(JNIEnv * env, qdb_ts_column_info * nativeColumns, size_t column_count, jobjectArray * columns) {
  jclass column_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_column_info");
  jmethodID constructor = env->GetMethodID(column_class, "<init>", "(Ljava/lang/String;I)V");

  *columns = env->NewObjectArray((jsize)column_count, column_class, NULL);

  for (size_t i = 0; i < column_count; i++) {
    env->SetObjectArrayElement(*columns, (jsize)i, env->NewObject(column_class,
                                                                  constructor,
                                                                  env->NewStringUTF(nativeColumns[i].name),
                                                                  nativeColumns[i].type));
  }
}

void
doublePointToNative(JNIEnv * env, jobject input, qdb_ts_double_point * native) {
  jfieldID timestampField, valueField;
  jclass objectClass;

  objectClass = env->GetObjectClass(input);

  timestampField = env->GetFieldID(objectClass, "timestamp", "Lnet/quasardb/qdb/jni/qdb_timespec;");
  valueField = env->GetFieldID(objectClass, "value", "D");

  timespecToNative(env, env->GetObjectField(input, timestampField), &(native->timestamp));
  native->value = env->GetDoubleField(input, valueField);
}

void
doublePointsToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_double_point * native) {
  qdb_ts_double_point * cur = native;
  for (size_t i = 0; i < count; ++i) {
    jobject point = (jobject)(env->GetObjectArrayElement(input, i));

    doublePointToNative(env, point, cur++);
  }
}

void
nativeToDoublePoint(JNIEnv * env, qdb_ts_double_point native, jobject * output) {
  jclass pointClass = env->FindClass("net/quasardb/qdb/jni/qdb_ts_double_point");
  jmethodID constructor = env->GetMethodID(pointClass, "<init>", "(Lnet/quasardb/qdb/jni/qdb_timespec;D)V");

  jobject timespec;
  nativeToTimespec(env, native.timestamp, &timespec);

  *output = env->NewObject(pointClass,
                           constructor,
                           timespec,
                           native.value);
}

void
nativeToDoublePoints(JNIEnv * env, qdb_ts_double_point * native, size_t count, jobjectArray * output) {
  jclass pointClass = env->FindClass("net/quasardb/qdb/jni/qdb_ts_double_point");

  *output = env->NewObjectArray((jsize)count, pointClass, NULL);

  for (size_t i = 0; i < count; i++) {
    jobject point;
    nativeToDoublePoint(env, native[i], &point);
    env->SetObjectArrayElement(*output, (jsize)i, point);
  }
}

void
blobPointToNative(JNIEnv * env, jobject input, qdb_ts_blob_point * native) {
  jfieldID timestampField, valueField;
  jclass objectClass;
  jobject value;

  objectClass = env->GetObjectClass(input);

  timestampField = env->GetFieldID(objectClass, "timestamp", "Lnet/quasardb/qdb/jni/qdb_timespec;");
  valueField = env->GetFieldID(objectClass, "value", "Ljava/nio/ByteBuffer;");
  value = env->GetObjectField(input, valueField);

  timespecToNative(env, env->GetObjectField(input, timestampField), &(native->timestamp));
  native->content = env->GetDirectBufferAddress(value);
  native->content_length = (qdb_size_t)env->GetDirectBufferCapacity(value);
}

void
blobPointsToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_blob_point * native) {
  qdb_ts_blob_point * cur = native;
  for (size_t i = 0; i < count; ++i) {
    jobject point = (jobject)(env->GetObjectArrayElement(input, i));

    blobPointToNative(env, point, cur++);
  }
}

void
nativeToByteBuffer(JNIEnv * env, void const * content, qdb_size_t contentLength, jobject * output) {
  *output = env->NewDirectByteBuffer((void *)(content), contentLength);
}

void
nativeToBlobPoint(JNIEnv * env, qdb_ts_blob_point native, jobject * output) {
  jclass pointClass = env->FindClass("net/quasardb/qdb/jni/qdb_ts_blob_point");
  jmethodID constructor = env->GetMethodID(pointClass, "<init>", "(Lnet/quasardb/qdb/jni/qdb_timespec;Ljava/nio/ByteBuffer;)V");

  jobject timespec, value;
  nativeToTimespec(env, native.timestamp, &timespec);
  nativeToByteBuffer(env, native.content, native.content_length, &value);

  *output = env->NewObject(pointClass,
                           constructor,
                           timespec,
                           value);
}

void
nativeToBlobPoints(JNIEnv * env, qdb_ts_blob_point * native, size_t count, jobjectArray * output) {
  jclass pointClass = env->FindClass("net/quasardb/qdb/jni/qdb_ts_blob_point");

  *output = env->NewObjectArray((jsize)count, pointClass, NULL);

  for (size_t i = 0; i < count; i++) {
    jobject point;
    nativeToBlobPoint(env, native[i], &point);
    env->SetObjectArrayElement(*output, (jsize)i, point);
  }
}

void
doubleAggregateToNative(JNIEnv *env, jobject input, qdb_ts_double_aggregation_t * native) {
  assert(input != NULL);

  jfieldID typeField, filteredRangeField, countField, resultField;
  jclass objectClass;

  objectClass = env->GetObjectClass(input);
  typeField = env->GetFieldID(objectClass, "aggregation_type", "J");
  filteredRangeField = env->GetFieldID(objectClass, "filtered_range", "Lnet/quasardb/qdb/jni/qdb_ts_filtered_range;");
  countField = env->GetFieldID(objectClass, "count", "J");
  resultField = env->GetFieldID(objectClass, "result", "Lnet/quasardb/qdb/jni/qdb_ts_double_point;");

  filteredRangeToNative(env, env->GetObjectField(input, filteredRangeField), &(native->filtered_range));
  doublePointToNative(env, env->GetObjectField(input, resultField), &(native->result));

  native->type = (qdb_ts_aggregation_type_t)(env->GetLongField(input, typeField));
  native->count = env->GetLongField(input, countField);
}

void
doubleAggregatesToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_double_aggregation_t * native) {
  assert(input != NULL);

  qdb_ts_double_aggregation_t * cur = native;
  for (size_t i = 0; i < count; ++i) {
    jobject aggregate = (jobject)(env->GetObjectArrayElement(input, i));

    doubleAggregateToNative(env, aggregate, cur++);
  }
}

void
nativeToDoubleAggregate(JNIEnv * env, qdb_ts_double_aggregation native, jobject * output) {
  jclass point_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_double_aggregation");
  jmethodID constructor = env->GetMethodID(point_class, "<init>", "(Lnet/quasardb/qdb/jni/qdb_ts_filtered_range;JJLnet/quasardb/qdb/jni/qdb_ts_double_point;)V");

  jobject filteredRange, result;

  nativeToFilteredRange(env, native.filtered_range, &filteredRange);
  nativeToDoublePoint(env, native.result, &result);

  jobject aggregate = env->NewObject(point_class,
                                     constructor,
                                     filteredRange,
                                     (jlong)native.type,
                                     (jlong)native.count,
                                     result);

  *output = aggregate;
}

void
nativeToDoubleAggregates(JNIEnv * env, qdb_ts_double_aggregation * native, size_t count, jobjectArray * output) {
  jclass aggregate_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_double_aggregation");
  assert (aggregate_class != NULL);

  *output = env->NewObjectArray((jsize)count, aggregate_class, NULL);

  for (size_t i = 0; i < count; i++) {
    jobject aggregate;

    nativeToDoubleAggregate(env, native[i], &aggregate);
    env->SetObjectArrayElement(*output, (jsize)i, aggregate);
  }
}

void
blobAggregateToNative(JNIEnv *env, jobject input, qdb_ts_blob_aggregation_t * native) {
  assert(input != NULL);

  jfieldID typeField, filteredRangeField, countField, resultField;
  jclass objectClass;

  objectClass = env->GetObjectClass(input);
  typeField = env->GetFieldID(objectClass, "aggregation_type", "J");
  filteredRangeField = env->GetFieldID(objectClass, "filtered_range", "Lnet/quasardb/qdb/jni/qdb_ts_filtered_range;");
  countField = env->GetFieldID(objectClass, "count", "J");
  resultField = env->GetFieldID(objectClass, "result", "Lnet/quasardb/qdb/jni/qdb_ts_blob_point;");

  filteredRangeToNative(env, env->GetObjectField(input, filteredRangeField), &(native->filtered_range));
  blobPointToNative(env, env->GetObjectField(input, resultField), &(native->result));

  native->type = (qdb_ts_aggregation_type_t)(env->GetLongField(input, typeField));
  native->count = env->GetLongField(input, countField);

  fflush(stdout);
}

void
blobAggregatesToNative(JNIEnv * env, jobjectArray input, size_t count, qdb_ts_blob_aggregation_t * native) {
  assert(input != NULL);

  qdb_ts_blob_aggregation_t * cur = native;
  for (size_t i = 0; i < count; ++i) {
    jobject aggregate = (jobject)(env->GetObjectArrayElement(input, i));

    blobAggregateToNative(env, aggregate, cur++);
  }
}

void
nativeToBlobAggregate(JNIEnv * env, qdb_ts_blob_aggregation native, jobject * output) {
  jclass point_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_blob_aggregation");
  jmethodID constructor = env->GetMethodID(point_class, "<init>", "(Lnet/quasardb/qdb/jni/qdb_ts_filtered_range;JJLnet/quasardb/qdb/jni/qdb_ts_blob_point;)V");

  jobject filteredRange, result;

  nativeToFilteredRange(env, native.filtered_range, &filteredRange);
  nativeToBlobPoint(env, native.result, &result);

  jobject aggregate = env->NewObject(point_class,
                                     constructor,
                                     filteredRange,
                                     (jlong)native.type,
                                     (jlong)native.count,
                                     result);

  *output = aggregate;
}

void
nativeToBlobAggregates(JNIEnv * env, qdb_ts_blob_aggregation * native, size_t count, jobjectArray * output) {
  jclass aggregate_class = env->FindClass("net/quasardb/qdb/jni/qdb_ts_blob_aggregation");
  assert (aggregate_class != NULL);

  *output = env->NewObjectArray((jsize)count, aggregate_class, NULL);

  for (size_t i = 0; i < count; i++) {
    jobject aggregate;

    nativeToBlobAggregate(env, native[i], &aggregate);
    env->SetObjectArrayElement(*output, (jsize)i, aggregate);
  }
}
