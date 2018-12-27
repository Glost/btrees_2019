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

namespace btree {

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

        void create(BaseBTree::TreeType treeType, UShort order, UShort keySize, const std::string& treeFileName);

        void createBTree(UShort order, UShort keySize, const std::string& treeFileName);

        void open(BaseBTree::TreeType treeType, const std::string& treeFileName);

        void close();

        void insert(const Byte* k) { tree->insert(k); }

        Byte* search(const Byte* k) { return tree->search(k); }

        int searchAll(const Byte* k, std::list<Byte*>& keys) { return tree->searchAll(k, keys); }

#ifdef BTREE_WITH_DELETION

        bool removeKey(const Byte* k) { return tree->remove(k); }

        int removeAll(const Byte* k) { return tree->removeAll(k); }

#endif

#ifdef __cplusplus
    }; // extern "C"
#endif

}; // namespace btree

#endif //BTREES_BTREE_C_H
