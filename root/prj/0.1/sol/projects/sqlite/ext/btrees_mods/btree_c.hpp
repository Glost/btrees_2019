/// \file
/// \brief     B-tree and modifications C-API.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      22.12.2018 -- 04.05.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "btree_c.h"

bool ByteComparator::compare(const Byte *lhv, const Byte *rhv, UInt sz)
{
    for (UInt i = 0; i < sz && i < firstPartBytes; ++i)
    {
        if (lhv[i] < rhv[i])
            return true;
        if (lhv[i] > rhv[i])
            return false;
    }

    return false;
}

bool ByteComparator::isEqual(const Byte *lhv, const Byte *rhv, UInt sz)
{
    for (UInt i = 0; i < sz && i < firstPartBytes; ++i)
        if (lhv[i] != rhv[i])
            return false;

    return true;
}

bool SearchAllByteComparator::compare(const Byte *lhv, const Byte *rhv, UInt sz)
{
    for (UInt i = 0; i < sz && i < firstPartBytes; ++i)
    {
        if (lhv[i] < rhv[i])
            return true;
        if (lhv[i] > rhv[i])
            return false;
    }

    return false;
}

std::string BytePrinter::print(const Byte *key, UInt sz)
{
    std::string result;

    for (UInt i = 0; i < sz && i < firstPartBytes && key[i] != 0; ++i)
        result += std::string(1, *((char*) &key[i]));

    result += std::string("; ") + std::to_string(*((long long*) &key[firstPartBytes]));

    return result;
}

#ifdef __cplusplus
extern "C" {
#endif

static void create(FileBaseBTree** pTree, BaseBTree::TreeType treeType,
        UShort order, UShort keySize, const char* treeFileName)
{
    close(pTree);

    try
    {
        *pTree = new FileBaseBTree(treeType, order, keySize, &byteComparator, treeFileName);
    }
    catch (std::runtime_error&)
    {
        *pTree = nullptr;
    }
}

static void createBTree(FileBaseBTree** pTree, UShort order, UShort keySize, const char* treeFileName)
{
    create(pTree, BaseBTree::TreeType::B_TREE, order, keySize, treeFileName);
}

static void open(FileBaseBTree** pTree, BaseBTree::TreeType treeType, const char* treeFileName)
{
    close(pTree);

    try
    {
        *pTree = new FileBaseBTree(treeType, treeFileName, &byteComparator);
    }
    catch (std::runtime_error&)
    {
        *pTree = nullptr;
    }
}

static void close(FileBaseBTree** pTree)
{
    if (*pTree != nullptr)
    {
        delete *pTree;
        *pTree = nullptr;
    }
}

static int searchAll(FileBaseBTree* tree, const Byte* k, Byte*** keysPointer)
{
    std::list<Byte*> keys;
    int result = tree->searchAll(k, keys);

    *keysPointer = (Byte**) malloc(sizeof(Byte*) * keys.size());

    std::list<Byte*>::iterator iter = keys.begin();
    for (int i = 0; i < keys.size() && iter != keys.end(); ++i, ++iter)
        (*keysPointer)[i] = *iter;

    return result;
}

static bool visualize(FileBaseBTree* tree, const char* dotFileName)
{
    std::ofstream dotFile(dotFileName);

    if (!dotFile.is_open())
        return false;

    tree->getTree()->writeDot(dotFile);

    dotFile.close();

    return true;
}

#ifdef __cplusplus
} // extern "C"
#endif
