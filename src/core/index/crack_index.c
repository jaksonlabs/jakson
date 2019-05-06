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

static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, struct crack_item *item, struct crack_index *index);
static void crack_item_drop(struct crack_item *item, struct crack_index *index);

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

static struct crack_item *crack_item_find(struct crack_index *index, u32 key)
{
        struct crack_item *item;

        for (item = index->lowest; key >= item->less_than_key; item = item->next)
                {}

        assert(item);
        assert(!item->prev || key >= item->prev->less_than_key);
        assert(key < item->less_than_key);

        if (item->values_uselist.num_elems > 25500) {
                struct crack_item *lhs, *rhs;
                u32 pivot = mean_key(item);
                assert(pivot < item->less_than_key);
                crack_item_split(&lhs, &rhs, pivot, item, index);
                item = key < pivot ? lhs : rhs;
        }

        assert(key < item->less_than_key);

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

        if (!vec_is_empty(&dst->values_freelist)) {
                slot = *(u32 *) vec_pop(&dst->values_freelist);
                value = vec_get(&dst->values, slot, struct crack_value);
        } else {
                slot = dst->values.num_elems;
                value = vec_new_and_get(&dst->values, struct crack_value);
        }

        crack_value_update(value, actual_key, data);
        vec_push(&dst->values_uselist, &slot, 1);
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
                                vec_push(&item->values_freelist, &pos, 1);
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
        if (likely(!vec_is_empty(&item->values_uselist))) {

                const void *result = crack_value_find_fist(item, key);
//                assert(result != NULL);
                return result;
        } else {
                return NULL;
        }
}

static struct crack_item *crack_item_new(struct crack_index *index)
{
        assert(index->crack_items.num_elems + 1 < UINT32_MAX);
        struct crack_item *item;
       // if (vec_is_empty(&index->crack_items_freelist)) {
                item = vec_new_and_get(&index->crack_items, struct crack_item);
        // } else {
        //      item = *(struct crack_item **) vec_pop(&index->crack_items_freelist);
        //}

        item->next = item->prev = NULL;
        item->less_than_key = UINT32_MAX;
        vec_create(&item->values_freelist, NULL, sizeof(u32), 100);
        vec_create(&item->values_uselist, NULL, sizeof(u32), 100);
        vec_create(&item->values, NULL, sizeof(struct crack_value), 100);

        return item;
}

ng5_func_unused
static void crack_item_split(struct crack_item **lhs, struct crack_item **rhs, u32 pivot, struct crack_item *item, struct crack_index *index)
{
        struct crack_item *lower = crack_item_new(index);
        struct crack_item *upper = crack_item_new(index);

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

        //assert(lower->values_uselist.num_elems);
        //assert(upper->values_uselist.num_elems);

        lower->less_than_key = pivot;
        upper->less_than_key = item->less_than_key;
        lower->next = upper;
        lower->prev = item->prev;
        upper->prev = lower;
        upper->next = item->next;

        if (item->prev) {
                item->prev->next = lower;
        }
        if (item->next) {
                item->next->prev = upper;
        }
        if (index->lowest == item) {
                index->lowest = lower;
        }

        *lhs = lower;
        *rhs = upper;
}

static void crack_item_drop(struct crack_item *item, struct crack_index *index)
{
        vec_drop(&item->values);
        vec_drop(&item->values_uselist);
        vec_drop(&item->values_freelist);

        ng5_unused(index);
        //vec_push(&index->crack_items_freelist, &item, 1);
        //ng5_zero_memory(item, sizeof(struct crack_item));
}


NG5_EXPORT(bool) crack_index_create(struct crack_index *index, struct err *err, u64 capacity)
{
        error_if_null(index)
        error_if_null(err)
        error_if_null(capacity)
        error_init(&index->err);
        vec_create(&index->crack_items, NULL, sizeof(struct crack_item), 1000);
        //vec_create(&index->crack_items_freelist, NULL, sizeof(struct crack_item *), 1000);
        crack_item_new(index);
        index->lowest = vec_get(&index->crack_items, 0, struct crack_item);

        return true;
}

NG5_EXPORT(bool) crack_index_drop(struct crack_index *index)
{
        error_if_null(index)

        struct crack_item *items = vec_all(&index->crack_items, struct crack_item);
        u32 num_items = index->crack_items.num_elems;
        while (num_items--) {
                crack_item_drop(items, index);
                items++;
        }
        vec_drop(&index->crack_items);
        //vec_drop(&index->crack_items_freelist);

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

NG5_EXPORT(const void *) crack_index_pop(struct crack_index *index, u32 key)
{
        error_if_null(index)

        struct crack_item *item = crack_item_find(index, key);
        const void *result = crack_item_pop(item, key);

        return result;
}