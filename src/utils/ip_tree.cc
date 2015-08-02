/*
 * ModSecurity, http://www.modsecurity.org/
 * Copyright (c) 2015 Trustwave Holdings, Inc. (http://www.trustwave.com/)
 *
 * You may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * If any of the files related to licensing are missing or if you have any
 * other questions related to licensing please contact Trustwave Holdings, Inc.
 * directly using the email address security@modsecurity.org.
 *
 */

#include "utils/ip_tree.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include <fstream>
#include <iostream>

#include "utils/geo_lookup.h"

namespace ModSecurity {
namespace Utils {

void IpTree::postOrderTraversal(TreeNode *node) {
    if (node == NULL) {
        return;
    }

    postOrderTraversal(node->left);
    postOrderTraversal(node->right);

    if (node->netmasks) {
        delete node->netmasks;
        node->netmasks = NULL;
    }
    if (node->prefix) {
        if (node->prefix->buffer) {
            delete node->prefix->buffer;
            node->prefix->buffer = NULL;
        }
        if (node->prefix->prefix_data) {
            delete node->prefix->prefix_data;
            node->prefix->prefix_data = NULL;
        }
        delete node->prefix;
        node->prefix = NULL;
    }
    delete node;
    node = NULL;
}

IpTree::~IpTree() {
    if (m_tree != NULL) {
        if (m_tree->ipv4_tree != NULL) {
            // Tree_traversal: Post-order to delete all the items.
            postOrderTraversal(m_tree->ipv4_tree->head);
            delete m_tree->ipv4_tree;
            m_tree->ipv4_tree = NULL;
        }
        if (m_tree->ipv6_tree != NULL) {
            // Tree_traversal: Post-order to delete all the items.
            postOrderTraversal(m_tree->ipv6_tree->head);
            delete m_tree->ipv6_tree;
            m_tree->ipv6_tree = NULL;
        }

        delete m_tree;
        m_tree = NULL;
    }
}


bool IpTree::addFromBuffer(const std::string& buffer, std::string *error) {
    char *error_msg = NULL;
    std::stringstream ss;
    std::string line;
    ss << buffer;
    int res = 0;

    for (std::string line; std::getline(ss, line); ) {
        res = ip_tree_from_param(buffer.c_str(), &m_tree, &error_msg);
        if (res != 0) {
            if (error_msg != NULL) {
                error->assign(error_msg);
            }
            return false;
        }
    }

    return true;
}


bool IpTree::contains(const std::string& ip) {
    int res = 0;
    char *error_msg = NULL;

    res = tree_contains_ip(m_tree, ip.c_str(), &error_msg);

    if (res < 0) {
        return false;
    }

    if (res > 0) {
        return true;
    }

    return false;
}


}  // namespace Utils
}  // namespace ModSecurity