/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 22.12.2018
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_BTREE_C_H
#define BTREES_BTREE_C_H

#include "btree.h"

using namespace btree;

struct ByteComparator : public BaseBTree::IComparator {

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz; ++i)
        {
            if (*lhv < *rhv)
                return true;
            if (*lhv > *rhv)
                return false;
        }

        return false;
    }

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz; ++i)
            if (*lhv != *rhv)
                return false;

        return true;
    }

}; // struct ByteComparator

#ifdef __cplusplus
extern "C" {
#endif

static FileBaseBTree* tree = nullptr;

static ByteComparator comparator;

static void create(BaseBTree::TreeType treeType, UShort order, UShort keySize, const char* treeFileName);

static void createBTree(UShort order, UShort keySize, const char* treeFileName);

static void open(BaseBTree::TreeType treeType, const char* treeFileName);

static void close();

static void insert(const Byte* k) { tree->insert(k); }

static Byte* search(const Byte* k) { return tree->search(k); }

static int searchAll(const Byte* k, std::list<Byte*>& keys) { return tree->searchAll(k, keys); }

#ifdef BTREE_WITH_DELETION

static bool removeKey(const Byte* k) { return tree->remove(k); }

static int removeAll(const Byte* k) { return tree->removeAll(k); }

#endif

#ifdef __cplusplus
}; // extern "C"
#endif

#include "btree_c.hpp"

#endif //BTREES_BTREE_C_H
