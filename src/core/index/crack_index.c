/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <std/sort.h>
#include "core/index/crack_index.h"

#define POSITION_UNUSED UINT32_MAX

static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, u32 victim_pos, struct crack_index *index);
static void crack_item_drop(struct crack_item *item);

ng5_func_unused
static u32 mean_key(struct crack_item *item)
{
        assert(item->values_uselist.num_elems);

        float mean = 0;
        u32 *positions = vec_all(&item->values_uselist, u32);
        u32 num_positions = item->values_uselist.num_elems;
        while (num_positions--) {
                u32 pos = *positions;
                if (pos != POSITION_UNUSED) {
                        const struct crack_value *value = vec_get(&item->values, pos, struct crack_value);
                        mean += value->actual_key;
                }
                positions++;
        }
        mean /= item->values_uselist.num_elems;
        return (u32) mean;
}

static int crack_item_cmp(const void *a, const void *b)
{
        struct crack_item *lhs = (struct crack_item *) a;
        struct crack_item *rhs = (struct crack_item *) b;
        if (rhs->less_than_key != UINT32_MAX) {
                return ((i64) lhs->less_than_key) - rhs->less_than_key;
        } else {
                return -1;
        }

}

ng5_func_unused
static struct crack_item *crack_item_search(const struct crack_item *begin, const struct crack_item *end,
        u32 key)
{
        intptr_t count = end - begin;
        intptr_t step;
        const struct crack_item *it;
        while (count > 0) {
                it = begin;
                step = count / 2;
                it += step;
                if (key + 1 >= it->less_than_key) {
                        begin = ++it;
                        count -= step + 1;
                }
                else
                        count = step;
        }
        return (struct crack_item *) begin;
}

static inline void crack_item_try_split(struct crack_item **item, u32 item_pos, u32 key, struct crack_index *index)
{
        struct crack_item *victim = *item;
        if (victim->values_uselist.num_elems > 2000) {
                struct crack_item *lhs, *rhs;
                u32 pivot = mean_key(victim);
                assert(pivot < victim->less_than_key);

                crack_item_split(&lhs, &rhs, pivot, item_pos, index);

                assert(key < lhs->less_than_key || key < rhs->less_than_key);
                assert(lhs->less_than_key < rhs->less_than_key);
                *item = key < pivot ? lhs : rhs;

#ifndef NDEBUG
                u32 num_broken = 0;
                for (u32 i = 0; i < ((i64) index->crack_items.num_elems) - 1; i++) {
                        struct crack_item *a = vec_get(&index->crack_items, i, struct crack_item);
                        struct crack_item *b = vec_get(&index->crack_items, i + 1, struct crack_item);
                        //assert(a->less_than_key <= b->less_than_key || b->less_than_key == UINT32_MAX);
                        if (!(a->less_than_key <= b->less_than_key)) {
                                ++num_broken;
                        }

                }
                if (num_broken > 0) {
                        fprintf(stderr, "(%0.4f%%)\n", (100*num_broken)/(float)index->crack_items.num_elems);
                }

#endif
        }
}

static struct crack_item *crack_item_find(struct crack_index *index, u32 key)
{
        u32 item_pos = 0;
        struct crack_item *all_items = vec_all(&index->crack_items, struct crack_item);
        struct crack_item *item;
        prefetch_read(all_items);

        for (; item_pos < index->crack_items.num_elems; item_pos++) {
                item = all_items + item_pos;
                if (unlikely(key < item->less_than_key)) {
                        break;
                }
        }
//
//        xxxxx++;

//        struct crack_item *found = crack_item_search(index->crack_items.base,
//                index->crack_items.base + (index->crack_items.num_elems - 1) * index->crack_items.elem_size,
//                key - 1);
//
//        i32 found_pos = found - all_items;
//        if (found_pos > 0) {
//                struct crack_item *x = vec_get(&index->crack_items, found_pos - 1, struct crack_item);
//                assert(x->less_than_key < found->less_than_key);
//                assert(key >= x->less_than_key);
//        }
//        if ((u32) found_pos < index->crack_items.num_elems - 1) {
//                struct crack_item *x = vec_get(&index->crack_items, found_pos + 1, struct crack_item);
//                assert(found->less_than_key < x->less_than_key);
//                assert(key < x->less_than_key);
//        }

//        while (key > found->less_than_key) {
//                found--;
//                found_pos--;
//                if (found_pos < 0) {
//                        found = vec_get(&index->crack_items, 1, struct crack_item);
//
//                        u32 item_pos = 0;
//                        for (; item_pos < index->crack_items.num_elems; item_pos++) {
//                                found = all_items + item_pos;
//                                if (unlikely(key < found->less_than_key)) {
//
//                                        break;
//                                }
//                        }
//
//                        break;
//                }
//        }
//        ng5_unused(found);
        //item = found;

        assert(key < item->less_than_key);

        assert(item);
        assert(key < item->less_than_key);
        assert(item <= all_items || (item - 1)->less_than_key < item->less_than_key );
     //   assert(item->less_than_key == UINT32_MAX || index->crack_items.num_elems < 1 || item->less_than_key < (item + 1)->less_than_key );

        crack_item_try_split(&item, item_pos, key, index);

        return item;
}

static void crack_value_update(struct crack_value *value, u32 actual_key, const void *data)
{
        assert(value);
        assert(data);
        value->actual_key = actual_key;
        value->value = data;
}

static void crack_item_add(struct crack_item *dst, const void *data, u32 actual_key)
{
        u32 slot;
        struct crack_value *value;

        if (unlikely(!vec_is_empty_unsafe(&dst->values_freelist))) {
                slot = *(u32 *) vec_pop_unsafe(&dst->values_freelist);
                value = vec_get(&dst->values, slot, struct crack_value);
        } else {
                slot = dst->values.num_elems;
                value = vec_new_and_get_unsafe(&dst->values, struct crack_value);
        }

        crack_value_update(value, actual_key, data);
        vec_push_inline(&dst->values_uselist, &slot, 1);
}

static const void *crack_value_find_fist(struct crack_item *item, u32 key)
{
        u32 *positions = vec_all(&item->values_uselist, u32);
        u32 num_positions = item->values_uselist.num_elems;

        while (num_positions--) {
                u32 pos = *positions;
                if (pos != POSITION_UNUSED) {
                        const struct crack_value *value = vec_get(&item->values, pos, struct crack_value);
                        if (key < value->actual_key) {
                                *positions = POSITION_UNUSED;
                                const void *result = value->value;
                                vec_push_inline(&item->values_freelist, &pos, 1);
                                //ng5_zero_memory(value, sizeof(struct crack_value));
                                return result;
                        }
                }
                positions++;
        }
        return NULL;
}

static const void *crack_item_pop(struct crack_item *item, u32 key)
{
        if (likely(!vec_is_empty_unsafe(&item->values_uselist))) {



                const void *result = crack_value_find_fist(item, key);
//                assert(result != NULL);
                return result;
        } else {
                return NULL;
        }
}

#define INIT_CRACK_ITEM_VALUE_CAP 25500
#define INIT_CRACK_ITEM_VALUEs_USELIST_CAP 25500

static struct crack_item *crack_item_new(struct crack_index *index)
{
        assert(index->crack_items.num_elems + 1 < UINT32_MAX);
        struct crack_item *item;
        item = vec_new_and_get_unsafe(&index->crack_items, struct crack_item);
        item->less_than_key = UINT32_MAX;
        vec_create(&item->values_freelist, NULL, sizeof(u32), 100);
        vec_create(&item->values_uselist, NULL, sizeof(u32), INIT_CRACK_ITEM_VALUEs_USELIST_CAP);
        vec_create(&item->values, NULL, sizeof(struct crack_value), INIT_CRACK_ITEM_VALUE_CAP);

        return item;
}

ng5_func_unused
static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, u32 victim_pos, struct crack_index *index)
{
        struct crack_item *item = vec_get(&index->crack_items, victim_pos, struct crack_item);
        struct crack_item *lower = crack_item_new(index);       /* added at the end of the list */
        struct crack_item *upper = crack_item_new(index);       /* added at the end of the list */


        assert(item->values.num_elems <= item->values.cap_elems);
        assert(lower->values.num_elems <= lower->values.cap_elems);
        assert(upper->values.num_elems <= upper->values.cap_elems);

        u32 *positions = vec_all(&item->values_uselist, u32);
        u32 num_positions = item->values_uselist.num_elems;
        while (num_positions--) {
                u32 pos = *positions;
                assert(pos == POSITION_UNUSED || pos < item->values.num_elems);
                if (pos != POSITION_UNUSED) {
                        const struct crack_value *value = vec_get(&item->values, pos, struct crack_value);
                        struct crack_item *dst = value->actual_key < pivot ? lower : upper;
                        crack_item_add(dst, value->value, value->actual_key);
                }
                positions++;
        }



        lower->less_than_key = pivot;
        upper->less_than_key = item->less_than_key;
        assert(lower->less_than_key < upper->less_than_key);

        crack_item_drop(item);

        /* update-in-place sorting: replace victim with lower, move everything right to lower one place up and replace
         * that new slot with upper; afterwards shrink the index crack items vector */
        u32 lower_pos = victim_pos;
        u32 upper_pos = lower_pos + 1;
        vec_set(&index->crack_items, lower_pos, lower);
        vec_make_space(&index->crack_items, lower_pos, 1);
        upper = vec_get(&index->crack_items, index->crack_items.num_elems - 1, struct crack_item);
        vec_set(&index->crack_items, upper_pos, upper);
        vec_clear_start_from(&index->crack_items, vec_length_unsafe(&index->crack_items) - 2);

#ifndef NDEBUG
        lower = vec_get(&index->crack_items, lower_pos, struct crack_item);
        upper = vec_get(&index->crack_items, upper_pos, struct crack_item);
        assert(lower->less_than_key < upper->less_than_key);
#endif

        *lhs = vec_get(&index->crack_items, lower_pos, struct crack_item);
        *rhs = vec_get(&index->crack_items, upper_pos, struct crack_item);

}

static void crack_item_drop(struct crack_item *item)
{
        vec_drop(&item->values);
        vec_drop(&item->values_uselist);
        vec_drop(&item->values_freelist);
}


NG5_EXPORT(bool) crack_index_create(struct crack_index *index, struct err *err, u64 capacity)
{
        error_if_null(index)
        error_if_null(err)
        error_if_null(capacity)
        error_init(&index->err);
        vec_create(&index->crack_items, NULL, sizeof(struct crack_item), 1000);
        crack_item_new(index);

        return true;
}

NG5_EXPORT(bool) crack_index_drop(struct crack_index *index)
{
        error_if_null(index)

        struct crack_item *items = vec_all(&index->crack_items, struct crack_item);

        for (u32 item_idx = 0; item_idx < index->crack_items.num_elems; item_idx++) {
                crack_item_drop(items);
                items++;
        }
        vec_drop(&index->crack_items);

        return true;
}

NG5_EXPORT(bool) crack_index_push(struct crack_index *index, u32 key, const void *value)
{
        error_if_null(index)
        error_if_null(value)

        struct crack_item *item = crack_item_find(index, key);
        assert(key < item->less_than_key);
        crack_item_add(item, value, key);

        return true;
}

NG5_EXPORT(bool) crack_index_get_counters(struct crack_index_counters *counters, const struct crack_index *index)
{
        error_if_null(counters);
        error_if_null(index);

        ng5_zero_memory(counters, sizeof(struct crack_index_counters));

        counters->index_first_level_items = index->crack_items.num_elems;
        for (u32 i = 0; i < index->crack_items.num_elems; i++) {
                struct crack_item *item = vec_get(&index->crack_items, i, struct crack_item);
                u32 is = item->values_uselist.num_elems;
                counters->index_min_2nd_level_items = ng5_min(counters->index_min_2nd_level_items, is);
                counters->index_max_2nd_level_items = ng5_max(counters->index_max_2nd_level_items, is);
                counters->index_avg_2nd_level_items += is;
        }

        counters->index_avg_2nd_level_items /= index->crack_items.num_elems;

        return true;
}

NG5_EXPORT(const void *) crack_index_pop(struct crack_index *index, u32 key)
{
        error_if_null(index)


        /* A 'pop' operation may remove elements from a crack item such that the pivot computation leads
         * to a number that destroys the order just rebuilt within 'crack_item_split'. Scan for this
         * case and adjust it if needed. */

        for (u32 i = 0; index->crack_items.num_elems > 2 && i < ((i64) index->crack_items.num_elems) - 1; i++) {
                struct crack_item *a = vec_get(&index->crack_items, i, struct crack_item);
                struct crack_item *b = vec_get(&index->crack_items, i + 1, struct crack_item);
                if (a->less_than_key > b->less_than_key) {
                        qsort(index->crack_items.base, index->crack_items.num_elems - 1, sizeof(struct crack_item), crack_item_cmp);
                        break;
                }
        }

        struct crack_item *item = crack_item_find(index, key);
        const void *result = crack_item_pop(item, key);

        return result;
}