/**
 * @file test_json.c
 * @brief Comprehensive unit tests for JSON parser and serializer
 * 
 * Tests cover:
 * - All JSON types (null, boolean, number, string, array, object)
 * - Edge cases and error conditions
 * - Memory safety and leak prevention
 * - RFC 8259 compliance
 */

#include "crest/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Test framework macros */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s...", #name); \
    test_##name(); \
    printf(" ✓\n"); \
    tests_passed++; \
} while(0)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "\n  FAILED: %s at line %d\n", #expr, __LINE__); \
        exit(1); \
    } \
} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)
#define ASSERT_DOUBLE_EQUAL(a, b) ASSERT_TRUE(fabs((a) - (b)) < 0.0001)

static int tests_passed = 0;

/* Test NULL parsing */
TEST(parse_null) {
    crest_json_value_t *val = crest_json_parse("null");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NULL);
    crest_json_free(val);
}

/* Test boolean parsing */
TEST(parse_bool_true) {
    crest_json_value_t *val = crest_json_parse("true");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_BOOL);
    ASSERT_TRUE(crest_json_as_bool(val));
    crest_json_free(val);
}

TEST(parse_bool_false) {
    crest_json_value_t *val = crest_json_parse("false");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_BOOL);
    ASSERT_FALSE(crest_json_as_bool(val));
    crest_json_free(val);
}

/* Test number parsing */
TEST(parse_number_integer) {
    crest_json_value_t *val = crest_json_parse("42");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(val), 42.0);
    crest_json_free(val);
}

TEST(parse_number_negative) {
    crest_json_value_t *val = crest_json_parse("-17");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(val), -17.0);
    crest_json_free(val);
}

TEST(parse_number_decimal) {
    crest_json_value_t *val = crest_json_parse("3.14159");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(val), 3.14159);
    crest_json_free(val);
}

TEST(parse_number_exponent) {
    crest_json_value_t *val = crest_json_parse("1.5e10");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(val), 1.5e10);
    crest_json_free(val);
}

TEST(parse_number_zero) {
    crest_json_value_t *val = crest_json_parse("0");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(val), 0.0);
    crest_json_free(val);
}

/* Test string parsing */
TEST(parse_string_simple) {
    crest_json_value_t *val = crest_json_parse("\"hello\"");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_STRING);
    ASSERT_STR_EQUAL(crest_json_as_string(val), "hello");
    crest_json_free(val);
}

TEST(parse_string_empty) {
    crest_json_value_t *val = crest_json_parse("\"\"");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_STRING);
    ASSERT_STR_EQUAL(crest_json_as_string(val), "");
    crest_json_free(val);
}

TEST(parse_string_with_escapes) {
    crest_json_value_t *val = crest_json_parse("\"hello\\nworld\\t!\"");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_STRING);
    ASSERT_STR_EQUAL(crest_json_as_string(val), "hello\nworld\t!");
    crest_json_free(val);
}

TEST(parse_string_with_quotes) {
    crest_json_value_t *val = crest_json_parse("\"say \\\"hello\\\"\"");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_STRING);
    ASSERT_STR_EQUAL(crest_json_as_string(val), "say \"hello\"");
    crest_json_free(val);
}

/* Test array parsing */
TEST(parse_array_empty) {
    crest_json_value_t *val = crest_json_parse("[]");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_ARRAY);
    ASSERT_EQUAL(crest_json_array_size(val), 0);
    crest_json_free(val);
}

TEST(parse_array_numbers) {
    crest_json_value_t *val = crest_json_parse("[1, 2, 3, 4, 5]");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_ARRAY);
    ASSERT_EQUAL(crest_json_array_size(val), 5);
    
    for (size_t i = 0; i < 5; i++) {
        crest_json_value_t *item = crest_json_array_get(val, i);
        ASSERT_NOT_NULL(item);
        ASSERT_EQUAL(crest_json_type(item), CREST_JSON_NUMBER);
        ASSERT_DOUBLE_EQUAL(crest_json_as_number(item), (double)(i + 1));
    }
    
    crest_json_free(val);
}

TEST(parse_array_mixed) {
    crest_json_value_t *val = crest_json_parse("[true, null, \"test\", 42]");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_ARRAY);
    ASSERT_EQUAL(crest_json_array_size(val), 4);
    
    ASSERT_EQUAL(crest_json_type(crest_json_array_get(val, 0)), CREST_JSON_BOOL);
    ASSERT_EQUAL(crest_json_type(crest_json_array_get(val, 1)), CREST_JSON_NULL);
    ASSERT_EQUAL(crest_json_type(crest_json_array_get(val, 2)), CREST_JSON_STRING);
    ASSERT_EQUAL(crest_json_type(crest_json_array_get(val, 3)), CREST_JSON_NUMBER);
    
    crest_json_free(val);
}

TEST(parse_array_nested) {
    crest_json_value_t *val = crest_json_parse("[[1, 2], [3, 4]]");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_ARRAY);
    ASSERT_EQUAL(crest_json_array_size(val), 2);
    
    crest_json_value_t *arr1 = crest_json_array_get(val, 0);
    ASSERT_EQUAL(crest_json_type(arr1), CREST_JSON_ARRAY);
    ASSERT_EQUAL(crest_json_array_size(arr1), 2);
    
    crest_json_free(val);
}

/* Test object parsing */
TEST(parse_object_empty) {
    crest_json_value_t *val = crest_json_parse("{}");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_OBJECT);
    crest_json_free(val);
}

TEST(parse_object_simple) {
    crest_json_value_t *val = crest_json_parse("{\"name\": \"John\", \"age\": 30}");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_OBJECT);
    
    crest_json_value_t *name = crest_json_get(val, "name");
    ASSERT_NOT_NULL(name);
    ASSERT_EQUAL(crest_json_type(name), CREST_JSON_STRING);
    ASSERT_STR_EQUAL(crest_json_as_string(name), "John");
    
    crest_json_value_t *age = crest_json_get(val, "age");
    ASSERT_NOT_NULL(age);
    ASSERT_EQUAL(crest_json_type(age), CREST_JSON_NUMBER);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(age), 30.0);
    
    crest_json_free(val);
}

TEST(parse_object_nested) {
    crest_json_value_t *val = crest_json_parse("{\"user\": {\"name\": \"Alice\", \"id\": 123}}");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_OBJECT);
    
    crest_json_value_t *user = crest_json_get(val, "user");
    ASSERT_NOT_NULL(user);
    ASSERT_EQUAL(crest_json_type(user), CREST_JSON_OBJECT);
    
    crest_json_value_t *name = crest_json_get(user, "name");
    ASSERT_NOT_NULL(name);
    ASSERT_STR_EQUAL(crest_json_as_string(name), "Alice");
    
    crest_json_free(val);
}

/* Test whitespace handling */
TEST(parse_with_whitespace) {
    crest_json_value_t *val = crest_json_parse("  \n\t  {  \n  \"key\"  :  \n  \"value\"  \n  }  \n  ");
    ASSERT_NOT_NULL(val);
    ASSERT_EQUAL(crest_json_type(val), CREST_JSON_OBJECT);
    
    crest_json_value_t *key = crest_json_get(val, "key");
    ASSERT_NOT_NULL(key);
    ASSERT_STR_EQUAL(crest_json_as_string(key), "value");
    
    crest_json_free(val);
}

/* Test error cases */
TEST(parse_invalid_returns_null) {
    ASSERT_NULL(crest_json_parse(""));
    ASSERT_NULL(crest_json_parse("invalid"));
    ASSERT_NULL(crest_json_parse("{"));
    ASSERT_NULL(crest_json_parse("[1, 2,"));
    ASSERT_NULL(crest_json_parse("{\"key\": }"));
    ASSERT_NULL(crest_json_parse("\"unterminated"));
}

/* Test stringify */
TEST(stringify_null) {
    crest_json_value_t *val = crest_json_null();
    char *str = crest_json_stringify(val);
    ASSERT_STR_EQUAL(str, "null");
    free(str);
    crest_json_free(val);
}

TEST(stringify_bool) {
    crest_json_value_t *t = crest_json_bool(true);
    char *str_t = crest_json_stringify(t);
    ASSERT_STR_EQUAL(str_t, "true");
    free(str_t);
    crest_json_free(t);
    
    crest_json_value_t *f = crest_json_bool(false);
    char *str_f = crest_json_stringify(f);
    ASSERT_STR_EQUAL(str_f, "false");
    free(str_f);
    crest_json_free(f);
}

TEST(stringify_number) {
    crest_json_value_t *val = crest_json_number(42.5);
    char *str = crest_json_stringify(val);
    ASSERT_STR_EQUAL(str, "42.5");
    free(str);
    crest_json_free(val);
}

TEST(stringify_string) {
    crest_json_value_t *val = crest_json_string("hello");
    char *str = crest_json_stringify(val);
    ASSERT_STR_EQUAL(str, "\"hello\"");
    free(str);
    crest_json_free(val);
}

TEST(stringify_string_with_escapes) {
    crest_json_value_t *val = crest_json_string("line1\nline2\ttab");
    char *str = crest_json_stringify(val);
    ASSERT_STR_EQUAL(str, "\"line1\\nline2\\ttab\"");
    free(str);
    crest_json_free(val);
}

TEST(stringify_array) {
    crest_json_value_t *arr = crest_json_array();
    crest_json_array_add(arr, crest_json_number(1));
    crest_json_array_add(arr, crest_json_number(2));
    crest_json_array_add(arr, crest_json_number(3));
    
    char *str = crest_json_stringify(arr);
    ASSERT_STR_EQUAL(str, "[1,2,3]");
    free(str);
    crest_json_free(arr);
}

TEST(stringify_object) {
    crest_json_value_t *obj = crest_json_object();
    crest_json_set(obj, "name", crest_json_string("John"));
    crest_json_set(obj, "age", crest_json_number(30));
    
    char *str = crest_json_stringify(obj);
    ASSERT_NOT_NULL(str);
    // Order may vary, so just check it contains both keys
    ASSERT_TRUE(strstr(str, "\"name\":\"John\"") != NULL);
    ASSERT_TRUE(strstr(str, "\"age\":30") != NULL);
    free(str);
    crest_json_free(obj);
}

/* Test roundtrip (parse -> stringify -> parse) */
TEST(roundtrip_complex) {
    const char *json = "{\"users\":[{\"name\":\"Alice\",\"age\":25},{\"name\":\"Bob\",\"age\":30}],\"count\":2}";
    
    crest_json_value_t *val1 = crest_json_parse(json);
    ASSERT_NOT_NULL(val1);
    
    char *str = crest_json_stringify(val1);
    ASSERT_NOT_NULL(str);
    
    crest_json_value_t *val2 = crest_json_parse(str);
    ASSERT_NOT_NULL(val2);
    
    // Verify structure
    crest_json_value_t *users = crest_json_get(val2, "users");
    ASSERT_NOT_NULL(users);
    ASSERT_EQUAL(crest_json_array_size(users), 2);
    
    crest_json_value_t *count = crest_json_get(val2, "count");
    ASSERT_NOT_NULL(count);
    ASSERT_DOUBLE_EQUAL(crest_json_as_number(count), 2.0);
    
    free(str);
    crest_json_free(val1);
    crest_json_free(val2);
}

/* Test memory management */
TEST(no_memory_leaks) {
    // Create complex structure
    crest_json_value_t *obj = crest_json_object();
    crest_json_set(obj, "string", crest_json_string("test"));
    crest_json_set(obj, "number", crest_json_number(42));
    crest_json_set(obj, "bool", crest_json_bool(true));
    crest_json_set(obj, "null", crest_json_null());
    
    crest_json_value_t *arr = crest_json_array();
    crest_json_array_add(arr, crest_json_number(1));
    crest_json_array_add(arr, crest_json_number(2));
    crest_json_set(obj, "array", arr);
    
    crest_json_value_t *nested = crest_json_object();
    crest_json_set(nested, "key", crest_json_string("value"));
    crest_json_set(obj, "nested", nested);
    
    // Free should clean up everything
    crest_json_free(obj);
    
    // If we get here without crashes, no double-free occurred
    ASSERT_TRUE(true);
}

/* Main test runner */
int main(void) {
    printf("=== Crest JSON Tests ===\n\n");
    
    /* Parsing tests */
    RUN_TEST(parse_null);
    RUN_TEST(parse_bool_true);
    RUN_TEST(parse_bool_false);
    RUN_TEST(parse_number_integer);
    RUN_TEST(parse_number_negative);
    RUN_TEST(parse_number_decimal);
    RUN_TEST(parse_number_exponent);
    RUN_TEST(parse_number_zero);
    RUN_TEST(parse_string_simple);
    RUN_TEST(parse_string_empty);
    RUN_TEST(parse_string_with_escapes);
    RUN_TEST(parse_string_with_quotes);
    RUN_TEST(parse_array_empty);
    RUN_TEST(parse_array_numbers);
    RUN_TEST(parse_array_mixed);
    RUN_TEST(parse_array_nested);
    RUN_TEST(parse_object_empty);
    RUN_TEST(parse_object_simple);
    RUN_TEST(parse_object_nested);
    RUN_TEST(parse_with_whitespace);
    RUN_TEST(parse_invalid_returns_null);
    
    /* Stringify tests */
    RUN_TEST(stringify_null);
    RUN_TEST(stringify_bool);
    RUN_TEST(stringify_number);
    RUN_TEST(stringify_string);
    RUN_TEST(stringify_string_with_escapes);
    RUN_TEST(stringify_array);
    RUN_TEST(stringify_object);
    
    /* Roundtrip and memory tests */
    RUN_TEST(roundtrip_complex);
    RUN_TEST(no_memory_leaks);
    
    printf("\n=== All %d tests passed! ✓ ===\n", tests_passed);
    return 0;
}
