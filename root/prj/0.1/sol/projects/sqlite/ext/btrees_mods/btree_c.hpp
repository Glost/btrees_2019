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

static void create(FileBaseBTree** pTree, BaseBTree::TreeType treeType,
        UShort order, UShort keySize, const char* treeFileName)
{
    close(pTree);

    *pTree = new FileBaseBTree(treeType, order, keySize, &byteComparator, treeFileName);
}

static void createBTree(FileBaseBTree** pTree, UShort order, UShort keySize, const char* treeFileName)
{
    create(pTree, BaseBTree::TreeType::B_TREE, order, keySize, treeFileName);
}

static void open(FileBaseBTree** pTree, BaseBTree::TreeType treeType, const char* treeFileName)
{
    close(pTree);

    *pTree = new FileBaseBTree(treeType, treeFileName, &byteComparator);
}

static void close(FileBaseBTree** pTree)
{
    if (*pTree != nullptr)
    {
        delete *pTree;
        *pTree = nullptr;
    }
}

static int searchAll(FileBaseBTree** pTree, const Byte* k, Byte*** keysPointer)
{
    std::list<Byte*> keys;
    int result = (*pTree)->searchAll(k, keys);

    *keysPointer = (Byte**) malloc(sizeof(Byte*) * keys.size());

    std::list<Byte*>::iterator iter = keys.begin();
    for (int i = 0; i < keys.size() && iter != keys.end(); ++i, ++iter)
        (*keysPointer)[i] = *iter;

    return result;
}

#ifdef __cplusplus
} // extern "C"
#endif
