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

#include "core/index/crack_index.h"

#define POSITION_UNUSED UINT32_MAX

static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, u32 item_idx, struct crack_index *index);
static void crack_item_drop(struct crack_item *item, u32 item_idx, struct crack_index *index);
static void crack_item_clear(struct crack_item *item, u32 item_idx, struct crack_index *index);

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
                        const struct crack_value *value = vec_get_unsafe(&item->values, pos, struct crack_value);
                        mean += value->actual_key;
                }
                positions++;
        }
        mean /= item->values_uselist.num_elems;
        return (u32) mean;
}

static inline void crack_item_try_split(struct crack_item **item, u32 key, struct crack_index *index)
{
        struct crack_item *victim = *item;
        if (victim->values_uselist.num_elems > 25500) {
                struct crack_item *lhs, *rhs;
                u32 pivot = mean_key(victim);
                assert(pivot < victim->less_than_key);
                crack_item_split(&lhs, &rhs, pivot, victim->self_idx, index);
                *item = key < pivot ? lhs : rhs;
        }

        assert(key < victim->less_than_key);
}

static struct crack_item *crack_item_find(struct crack_index *index, u32 key)
{
        struct crack_item *item;

        for (item = (vec_get_unsafe(&index->crack_items, index->lowest_item_id, struct crack_item));
             key >= item->less_than_key;
             item = (vec_get_unsafe(&index->crack_items, item->next_idx, struct crack_item)))
                {}
        

        assert(item);
        //assert(!item->prev || key >= item->prev->less_than_key);
        assert(key < item->less_than_key);

        crack_item_try_split(&item, key, index);

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
                value = vec_get_unsafe(&dst->values, slot, struct crack_value);
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
                        const struct crack_value *value = vec_get_unsafe(&item->values, pos, struct crack_value);
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
        u32 self_index;
        struct crack_item *item;
        if (vec_is_empty_unsafe(&index->crack_items_freelist)) {
                self_index = index->crack_items.num_elems;
                item = vec_new_and_get_unsafe(&index->crack_items, struct crack_item);
        } else {
                self_index = *(u32 *) vec_pop(&index->crack_items_freelist);
                item = vec_get_unsafe(&index->crack_items, self_index, struct crack_item);
        }

        item->self_idx = self_index;
        item->next_idx = item->prev_idx = UINT32_MAX;
        item->less_than_key = UINT32_MAX;
        vec_create(&item->values_freelist, NULL, sizeof(u32), 100);
        vec_create(&item->values_uselist, NULL, sizeof(u32), INIT_CRACK_ITEM_VALUEs_USELIST_CAP);
        vec_create(&item->values, NULL, sizeof(struct crack_value), INIT_CRACK_ITEM_VALUE_CAP);

        return item;
}

ng5_func_unused
static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, u32 item_idx, struct crack_index *index)
{
        u32 lower_idx = crack_item_new(index)->self_idx;
        struct crack_item *upper = crack_item_new(index);
        struct crack_item *item = vec_get_unsafe(&index->crack_items, item_idx, struct crack_item);
        struct crack_item *lower = vec_get_unsafe(&index->crack_items, lower_idx, struct crack_item);

        assert(item->values.num_elems <= item->values.cap_elems);
        assert(lower->values.num_elems <= lower->values.cap_elems);
        assert(upper->values.num_elems <= upper->values.cap_elems);

        u32 *positions = vec_all(&item->values_uselist, u32);
        u32 num_positions = item->values_uselist.num_elems;
        while (num_positions--) {
                u32 pos = *positions;
                assert(pos == POSITION_UNUSED || pos < item->values.num_elems);
                if (pos != POSITION_UNUSED) {
                        const struct crack_value *value = vec_get_unsafe(&item->values, pos, struct crack_value);
                        struct crack_item *dst = value->actual_key < pivot ? lower : upper;
                        crack_item_add(dst, value->value, value->actual_key);
                }
                positions++;
        }

        //assert(lower->values_uselist.num_elems);
        //assert(upper->values_uselist.num_elems);

        lower->less_than_key = pivot;
        upper->less_than_key = item->less_than_key;
        lower->next_idx = upper->self_idx;
        lower->prev_idx = item->prev_idx;
        upper->prev_idx = lower->self_idx;
        upper->next_idx = item->next_idx;

        if (item->prev_idx != UINT32_MAX) {
                (vec_get_unsafe(&index->crack_items, item->prev_idx, struct crack_item))->next_idx = lower->self_idx;
        }
        if (item->next_idx != UINT32_MAX) {
                (vec_get_unsafe(&index->crack_items, item->next_idx, struct crack_item))->prev_idx = upper->self_idx;
        }
        if (index->lowest_item_id == item_idx) {
                index->lowest_item_id = lower_idx;
        }

        *lhs = lower;
        *rhs = upper;

        crack_item_clear(item, item_idx, index);
}

static void crack_item_clear(struct crack_item *item, u32 item_idx, struct crack_index *index)
{
        vec_clear(&item->values);
        vec_clear(&item->values_uselist);
        vec_clear(&item->values_freelist);
        vec_push_inline(&index->crack_items_freelist, &item_idx, 1);
};

static void crack_item_drop(struct crack_item *item, u32 item_idx, struct crack_index *index)
{
        vec_drop(&item->values);
        vec_drop(&item->values_uselist);
        vec_drop(&item->values_freelist);
        vec_push_inline(&index->crack_items_freelist, &item_idx, 1);
}


NG5_EXPORT(bool) crack_index_create(struct crack_index *index, struct err *err, u64 capacity)
{
        error_if_null(index)
        error_if_null(err)
        error_if_null(capacity)
        error_init(&index->err);
        vec_create(&index->crack_items, NULL, sizeof(struct crack_item), 1000);
        vec_create(&index->crack_items_freelist, NULL, sizeof(u32), 1000);
        crack_item_new(index);
        index->lowest_item_id = 0;//vec_get(&index->crack_items, 0, struct crack_item);

        return true;
}

NG5_EXPORT(bool) crack_index_drop(struct crack_index *index)
{
        error_if_null(index)

        struct crack_item *items = vec_all(&index->crack_items, struct crack_item);

        for (u32 item_idx = 0; item_idx < index->crack_items.num_elems; item_idx++) {
                crack_item_drop(items, item_idx, index);
                items++;
        }
        vec_drop(&index->crack_items);
        vec_drop(&index->crack_items_freelist);

        return true;
}

NG5_EXPORT(bool) crack_index_push(struct crack_index *index, u32 key, const void *value)
{
        error_if_null(index)
        error_if_null(value)

        struct crack_item *item = crack_item_find(index, key);
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
                struct crack_item *item = vec_get_unsafe(&index->crack_items, i, struct crack_item);
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

        struct crack_item *item = crack_item_find(index, key);
        const void *result = crack_item_pop(item, key);

        return result;
}