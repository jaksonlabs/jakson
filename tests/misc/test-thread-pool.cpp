#include <gtest/gtest.h>
#include <atomic>
#include <cmath>
#include <fstream>

#include <jakson/utils/priority_queue.h>
#include <jakson/std/thread_pool.h>
#include <jakson/std/thread_pool/monitor.h>

// Test if push and isEmpty works correkt
TEST(PriorityQueue, IsEmpty)
{
        priority_queue queue;
        priority_queue_init(&queue);

        EXPECT_EQ(priority_queue_is_empty(&queue), 1);

        for (size_t i = 0; i < 10; ++i) {
                priority_queue_push(&queue, (void *) i, i);
        }

        EXPECT_EQ(!priority_queue_is_empty(&queue), 1);

        priority_queue_free(&queue);
}

// Test if push with Prio works correktly
TEST(PriorityQueue, PushPrio)
{
        priority_queue queue;
        priority_queue_init(&queue);

        for (size_t i = 1; i < 5; ++i) {
                priority_queue_push(&queue, (void *) i, i);
        }

        int test = 99;
        priority_queue_push(&queue, (void *) &test, 0);

        EXPECT_EQ(*(int *) priority_queue_pop(&queue), test);

        priority_queue_free(&queue);
}

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60932
// using atomic_int = std::atomic<int>;
#define CPP_TEST 1

// Test the creation of thread pools
TEST(ThreadPool, CreateThreadPool)
{
        thread_pool *pool = thread_pool_create(2, 0);
        EXPECT_TRUE(pool);

        thread_pool_free(pool);
}

// Test the creation of group Ids of thread pools
TEST(ThreadPool, NAME)
{
        thread_pool *pool = thread_pool_create_named(2, "ThreadPool", 0);

        EXPECT_TRUE(pool);
        ASSERT_STREQ(pool->name, "ThreadPool");

        thread_pool_free(pool);
}

void work(void *args)
{
        int *res = (int *) args;

        double exp = *res;

        double v = 0.0;
        for (double i = 0; i < 100024; ++i) {
                v += pow(i, 1 / exp);
        }

        for (; *res > 0; (*res)--) {

        }
}

TEST(ThreadPool, RESIZE)
{
        thread_pool *pool = thread_pool_create(4, 0);
        EXPECT_TRUE(pool);

        bool status = thread_pool_resize(pool, 6);

        EXPECT_EQ(status, true);
        EXPECT_EQ(pool->size, 6U);

        thread_pool_free(pool);
}

TEST(ThreadPool, WAIT)
{
        int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000};
        thread_pool *pool = thread_pool_create(2, 0);
        thread_task tasks[6];

        for (int i = 0; i < 6; i++) {
                tasks[i].args = (void *) &test[i];
                tasks[i].routine = work;
        }

        thread_pool_enqueue_tasks_wait(tasks, pool, 6);

        // All entries in the array have to be 0 to ensure the pool waits for all tasks
        for (int i = 0; i < 6; i++) {
                ASSERT_EQ(test[i], 0);
        }

        thread_pool_free(pool);
}

TEST(ThreadPool, WAIT_FOR_ALL)
{
        int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000};
        thread_pool *pool = thread_pool_create(2, 0);
        thread_task tasks[6];

        for (int i = 0; i < 6; i++) {
                tasks[i].args = (void *) &test[i];
                tasks[i].routine = work;
                thread_pool_enqueue_task(&tasks[i], pool, NULL);
        }

        thread_pool_wait_for_all(pool);

        // All entries in the array have to be 0 to ensure the pool waits for all tasks
        for (int i = 0; i < 6; i++) {
                ASSERT_EQ(test[i], 0);
        }

        thread_pool_free(pool);
}

void LIVE_RESIZE_work(void *args)
{
        int *res = static_cast<int *>(args);
        int sum = 0;
        for (int i = 0; i <= *res; ++i) {
                sum += i * i / 8;
        }

        *res = sum;
}

TEST(ThreadPool, LIVE_RESIZE)
{
        thread_pool *pool = thread_pool_create(2, 0);
        task_handle hndl;
        const int numThreads = 1 << 11;
        thread_task tasks[numThreads];
        int results[numThreads];
        for (int i = numThreads - 1; i >= 0; --i) {
                results[i] = i;
                tasks[i].args = &results[i];
                tasks[i].routine = LIVE_RESIZE_work;
                tasks[i].priority = 0;
                if (i == 0) { tasks[i].priority = 1; }
                thread_pool_enqueue_task(&tasks[i], pool, &hndl);
                if (i == numThreads * 1 / 3) {
                        ASSERT_EQ(pool->size, 3U);
                        thread_pool_resize(pool, 4);
                        ASSERT_EQ(pool->size, 4U);
                }
                if (i == numThreads * 2 / 3) {
                        ASSERT_EQ(pool->size, 4U);
                        thread_pool_resize(pool, 3);
                        ASSERT_EQ(pool->size, 3U);
                }
                if (i == numThreads * 3 / 4) {
                        ASSERT_EQ(pool->size, 2U);
                        thread_pool_resize(pool, 4);
                        ASSERT_EQ(pool->size, 4U);
                }
        }
        ASSERT_EQ(pool->size, 4U);

        thread_pool_wait_for_all(pool);
        thread_pool_free(pool);

        // verify results
        int sum = 0;
        for (int i = 0; i < numThreads; ++i) {
                sum += i * i / 8;
                ASSERT_EQ(results[i], sum);
        }
}

TEST(ThreadPool, TASK_STATISTICS)
{
        int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000};
        thread_pool *pool = thread_pool_create(2, 1);
        thread_task tasks[6];

        for (int i = 0; i < 6; i++) {
                tasks[i].args = (void *) &test[i];
                tasks[i].routine = work;
                thread_pool_enqueue_task(&tasks[i], pool, NULL);
        }

        thread_pool_wait_for_all(pool);
        thread_pool_free(pool);
}

//TEST(ThreadPool, POOL_STATISTICS)
//{
//      signal(SIGSEGV, __sig_seg);
//
//        const int number_task = 1000;
//        for (size_t counter = 2; counter < 32; ++counter) {
//                counter = 32;
//                int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000};
//                thread_pool *pool = thread_pool_create(counter, 1);
//                thread_task tasks[number_task];
//                thread_pool_stats pool_stats;
//
//                for (int i = 0; i < number_task; i++) {
//                        tasks[i].args = (void *) &test[i];
//                        tasks[i].routine = work;
//                        thread_pool_enqueue_task(&tasks[i], pool, NULL);
//                }
//
//                pool_stats = thread_pool_get_stats(pool);
//                ASSERT_EQ(pool_stats.task_enqueued_count, number_task);
//                ASSERT_NE(pool_stats.task_complete_count, number_task);
//
//                thread_pool_wait_for_all(pool);
//
//                pool_stats = thread_pool_get_stats(pool);
//                ASSERT_EQ(pool_stats.task_complete_count, pool_stats.task_enqueued_count);
//
//                thread_pool_free(pool);
//        }
//}

TEST(ThreadPool, THREAD_STATISTICS)
{
        int test[] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000};
        thread_pool *pool = thread_pool_create(2, 1);
        thread_task tasks[6];
        thread_stats thread_stats;

        for (int i = 0; i < 6; i++) {
                tasks[i].args = (void *) &test[i];
                tasks[i].routine = work;
        }

        thread_pool_enqueue_tasks_wait(tasks, pool, 6);

        //save to csv
        std::ofstream stats;
        stats.open("Statistics/threads.csv");

        stats << "BusyTime IdleTime" << "\n";

        for (size_t i = 0; i < pool->size; ++i) {
                thread_stats = thread_pool_get_thread_stats(pool, i);
                stats << thread_stats.busy_time << " " << thread_stats.idle_time << "\n";
        }
        stats.close();

        thread_pool_free(pool);
}

int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}