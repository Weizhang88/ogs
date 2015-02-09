/**
 * @file TestPntInElement.cpp
 * @author Karsten Rink
 * @date 2014-09-23
 * @brief Tests for check if a point is located inside an element
 *
 * @copyright
 * Copyright (c) 2012-2015, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include "gtest/gtest.h"

#include "Mesh.h"
#include "Node.h"
#include "MeshEditing/MeshRevision.h"
#include "Elements/Element.h"
#include "Elements/Tri.h"
#include "Elements/Quad.h"
#include "Elements/Tet.h"
#include "Elements/Hex.h"
#include "Elements/Pyramid.h"
#include "Elements/Prism.h"


std::vector<MeshLib::Node*> createNodes()
{
    std::vector<MeshLib::Node*> nodes;
    nodes.push_back(new MeshLib::Node(0,0,0));
    nodes.push_back(new MeshLib::Node(1,0,0));
    nodes.push_back(new MeshLib::Node(1,1,0));
    nodes.push_back(new MeshLib::Node(0,1,0));
    nodes.push_back(new MeshLib::Node(0,0,1));
    nodes.push_back(new MeshLib::Node(1,0,1));
    nodes.push_back(new MeshLib::Node(1,1,1));
    nodes.push_back(new MeshLib::Node(0,1,1));
    return nodes;
}

void deleteNodes(std::vector<MeshLib::Node*> &nodes)
{
    std::for_each(nodes.begin(), nodes.end(), [](MeshLib::Node* node){ delete node; });
}

TEST(IsPntInElement, Line)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 2> line_nodes = {{ nodes[0], nodes[4] }};
    MeshLib::Line line(line_nodes);
    pnt = GeoLib::Point(0,0,0.7);
    ASSERT_EQ(true, line.isPntInElement(pnt));
    pnt = GeoLib::Point(0,0.1,0.7);
    ASSERT_EQ(false, line.isPntInElement(pnt));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Tri)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 3> tri_nodes = {{ nodes[0], nodes[1], nodes[4] }};
    MeshLib::Tri tri(tri_nodes);

    pnt = GeoLib::Point(0.1,0,0.1);
    ASSERT_EQ(true, tri.isPntInElement(pnt));
    pnt = GeoLib::Point(0.9,0,0.7);
    ASSERT_EQ(false, tri.isPntInElement(pnt));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Quad)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 4> quad_nodes =
        {{ nodes[0], nodes[1], nodes[5], nodes[4] }};
    MeshLib::Quad quad(quad_nodes);

    pnt = GeoLib::Point(0.1,0,0.1);
    ASSERT_EQ(true, quad.isPntInElement(pnt));
    pnt = GeoLib::Point(0.999,0,0.001);
    ASSERT_EQ(true, quad.isPntInElement(pnt));
    pnt = GeoLib::Point(0.5,0.00001,1);
    ASSERT_EQ(false, quad.isPntInElement(pnt));
    ASSERT_EQ(true, quad.isPntInElement(pnt, 0.001));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Tet)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 4> tet_nodes =
        {{ nodes[0], nodes[1], nodes[2], nodes[4] }};
    MeshLib::Tet tet(tet_nodes);

    pnt = GeoLib::Point(0.5,0.3,0.1);
    ASSERT_EQ(true, tet.isPntInElement(pnt));
    pnt = GeoLib::Point(0.5,0.6,0.1);
    ASSERT_EQ(false, tet.isPntInElement(pnt));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Pyramid)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 5> pyr_nodes;
    std::copy(nodes.begin(), nodes.begin()+5, pyr_nodes.begin());
    MeshLib::Pyramid pyr(pyr_nodes);

    pnt = GeoLib::Point(0.5,0.00001,-0.000001);
    ASSERT_EQ(false, pyr.isPntInElement(pnt));
    ASSERT_EQ(true, pyr.isPntInElement(pnt, 0.0001));
    pnt = GeoLib::Point(0.5,0.5,0.1);
    ASSERT_EQ(true, pyr.isPntInElement(pnt));
    pnt = GeoLib::Point(0.5,0.5,0.51);
    ASSERT_EQ(false, pyr.isPntInElement(pnt));
    ASSERT_EQ(true, pyr.isPntInElement(pnt, 0.02));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Prism)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 6> prism_nodes =
        {{ nodes[0], nodes[1], nodes[2], nodes[4], nodes[5], nodes[6] }};
    MeshLib::Prism prism(prism_nodes);

    pnt = GeoLib::Point(0.5,0.5,0.1);
    ASSERT_EQ(true, prism.isPntInElement(pnt));
    pnt = GeoLib::Point(0.49,0.51,0.1);
    ASSERT_EQ(false, prism.isPntInElement(pnt));
    ASSERT_EQ(true, prism.isPntInElement(pnt, 0.03));
    deleteNodes(nodes);
}

TEST(IsPntInElement, Hex)
{
    GeoLib::Point pnt;
    std::vector<MeshLib::Node*> nodes (createNodes());
    std::array<MeshLib::Node*, 8> hex_nodes;
    std::copy(nodes.begin(), nodes.end(), hex_nodes.begin());
    MeshLib::Hex hex(hex_nodes);

    pnt = GeoLib::Point(0.99,0.99,0.99);
    ASSERT_EQ(true, hex.isPntInElement(pnt));
    pnt = GeoLib::Point(0.99,0,0);
    ASSERT_EQ(true, hex.isPntInElement(pnt));
    pnt = GeoLib::Point(0.0, 0.0, 0.0);
    ASSERT_EQ(true, hex.isPntInElement(pnt));
    pnt = GeoLib::Point(1.01,0.99,0.99);
    ASSERT_EQ(false, hex.isPntInElement(pnt));
    ASSERT_EQ(true, hex.isPntInElement(pnt, 0.02));
    deleteNodes(nodes);
}