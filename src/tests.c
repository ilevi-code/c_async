#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "generator.h"

int failed = 0;
#define EXPECT(cond)                              \
    ({                                            \
        if (!(cond)) {                            \
            printf("Expect failed: %s\n", #cond); \
            failed = 1;                           \
            return;                               \
        }                                         \
    })

void empty_func() {}
void expectes_param_deadbeefcafe(long long param)
{
    assert(param == 0xdeadbeefcafe);
}

void serires_yielder(int param)
{
    yield(1);
    yield(2);
    yield(3);
}

void test_generator_creation()
{
    struct generator* gen = generator_create(&empty_func);
    assert(gen != NULL);
    generator_destory(gen);
}

void test_empty_generator()
{
    struct generator* gen = generator_create(&empty_func);
    assert(gen != NULL);
    next(gen);
    EXPECT(gen->status == GEN_STATUS_DONE);
    generator_destory(gen);
}

void test_generator_iteration()
{
    struct generator* gen = generator_create(&serires_yielder);
    assert(gen != NULL);
    EXPECT(next(gen) == 1);
    generator_destory(gen);
}

void test_generator_exhuastion()
{
    int sum = 0;
    struct generator* gen = generator_create(&serires_yielder);
    assert(gen != NULL);
    int value;
    GENERATOR_WHILE(value, gen)
    {
        sum += value;
    }
    EXPECT(sum == 6);
    generator_destory(gen);
}

void test_generator_overuse()
{
    struct generator* gen = generator_create(&serires_yielder);
    assert(gen != NULL);
    int value;
    GENERATOR_WHILE(value, gen)
    {
        (void)value;
    }
    next(gen);  // this should do nothing
    EXPECT(gen->status == GEN_STATUS_DONE);
    generator_destory(gen);
}
void test_parametrized_generator()
{
    struct generator* gen = generator_create(&empty_func, 0xdeadbeefcafe);
    assert(gen != NULL);
    next(gen);
    EXPECT(gen->status == GEN_STATUS_DONE);
    generator_destory(gen);
}

void uses_other_generator(int reps)
{
    for (int i = 0; i < reps; i++) {
        struct generator* gen = generator_create(&serires_yielder, 0xdeadbeefcafe);
        assert(gen != NULL);
        int value;
        GENERATOR_WHILE(value, gen)
        {
            yield(value);
        }
        generator_destory(gen);
    }
}

void test_generator_recursion()
{
    int sum = 0;
    const int REPS = 10;

    struct generator* gen = generator_create(&uses_other_generator, REPS);
    assert(gen != NULL);

    int value;
    GENERATOR_WHILE(value, gen)
    {
        sum += value;
    }

    EXPECT(sum == 6 * REPS);
    generator_destory(gen);
}

#define RUN_TEST(test) run_test(test, #test);

void run_test(void (*test)(void), const char* test_name)
{
    printf("[ RUN    ] %s\n", test_name);
    failed = 0;
    test();
    if (failed) {
        printf("[  ERROR ] %s\n", test_name);
    } else {
        printf("[     OK ] %s\n", test_name);
    }
}

void run_tests()
{
    printf("[--------] Running tests...\n");
    RUN_TEST(test_generator_creation);
    RUN_TEST(test_empty_generator);
    RUN_TEST(test_generator_iteration);
    RUN_TEST(test_generator_exhuastion);
    RUN_TEST(test_generator_overuse);
    RUN_TEST(test_parametrized_generator);
    RUN_TEST(test_generator_recursion);
    printf("[--------] All tests passed!\n");
}
