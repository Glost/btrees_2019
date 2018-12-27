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

namespace btree {

#ifdef __cplusplus
    extern "C" {
#endif

    void create(BaseBTree::TreeType treeType, UShort order, UShort keySize, const std::string& treeFileName)
    {
        close();

        tree = new FileBaseBTree(treeType, order, keySize, &comparator, treeFileName);
    }

    void createBTree(UShort order, UShort keySize, const std::string& treeFileName)
    {
        create(BaseBTree::TreeType::B_TREE, order, keySize, treeFileName);
    }

    void open(BaseBTree::TreeType treeType, const std::string& treeFileName)
    {
        close();

        tree = new FileBaseBTree(treeType, treeFileName, &comparator);
    }

    void close()
    {
        if (tree != nullptr)
        {
            delete tree;
            tree = nullptr;
        }
    }

#ifdef __cplusplus
    } // extern "C"
#endif

} // namespace btree
