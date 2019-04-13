/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 22.12.2018
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "btree_c.h"

#ifdef __cplusplus
extern "C" {
#endif

static void create(BaseBTree::TreeType treeType, UShort order, UShort keySize, const char* treeFileName)
{
    close();

    tree = new FileBaseBTree(treeType, order, keySize, &firstPartByteComparator, treeFileName);
}

static void createBTree(UShort order, UShort keySize, const char* treeFileName)
{
    create(BaseBTree::TreeType::B_TREE, order, keySize, treeFileName);
}

static void open(BaseBTree::TreeType treeType, const char* treeFileName)
{
    close();

    tree = new FileBaseBTree(treeType, treeFileName, &firstPartByteComparator);
}

static void close()
{
    if (tree != nullptr)
    {
        delete tree;
        tree = nullptr;
    }
}

static int searchAll(const Byte* k, Byte*** keysPointer)
{
    std::list<Byte*> keys;
    int result = tree->searchAll(k, keys);

    *keysPointer = (Byte**) malloc(sizeof(Byte*) * keys.size());

    std::list<Byte*>::iterator iter = keys.begin();
    for (int i = 0; i < keys.size() && iter != keys.end(); ++i, ++iter)
        (*keysPointer)[i] = *iter;

    return result;
}

#ifdef __cplusplus
} // extern "C"
#endif
