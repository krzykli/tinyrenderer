
#ifndef __SCENEGRAPH_H__
#define __SCENEGRAPH_H__

#include "types.h"

void addNodes(Node* parent, Node* nodes, int count) {
    for (int i = 0; i < count; i++) {
        Node node = nodes[i];
        node.parent = parent;
        parent->children.push_back(&node);
    }
}

#endif // __SCENEGRAPH_H__
