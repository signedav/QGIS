"""QGIS Unit tests for QgsServer WMS with opaque groups.

From build dir, run: ctest -R PyQgsServerWMSOpaqueGroups -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Dave Signer"
__date__ = "11/04/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ["QT_HASH_SEED"] = "1"

import json
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET

import osgeo.gdal  # NOQA
from qgis.core import (
    Qgis,
    QgsAttributeEditorField,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMapLayer,
    QgsMemoryProviderUtils,
    QgsPointXY,
    QgsProject,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import QUrl, QVariant
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerRequest,
)
from qgis.testing import QgisTestCase, unittest
from test_qgsserver_wms import TestQgsServerWMSTestBase


class TestQgsServerWMSOpaqueGroups(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for several request with opaque groups"""

    """ Project to test
    - street-group
	- streetline (based on streetline)
	- streetarea (based on streetarea)
    - road-opaquegroup
        - roadline (based on roadline)
        - roadarea (based on roadarea)
    - way-opaquegroup-excludedparts
        - wayline (based on wayline)
        - wayarea-excluded (based on wayarea)
    - trafficsignpoint (based on point1)

    The street-group is a normal group. 
    The road-opaquegroup is an opaque group.
    The way-opaquegroup-excludedparts is an opaque group.
    The sign-opaquegroup-samenamed is an opaque group.
    The wayarea-excluded is excluded and should not be displayed on GetMap etc.
    """

    def setUp(self):
        super().setUp()
        self.project = os.path.join(self.testdata_path, "test_project_opaque.qgz")

    def tearDown(self):
        super().tearDown()
        os.putenv("QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS", "")

    def _get_layer(self, xml_result, layer_name):
        """
        Get a layer element by name. Parse XML once and return the layer element.
        """
        root = ET.fromstring(xml_result)
        for layer in root.iter("Layer"):
            name_elem = layer.find("Name")
            if name_elem is not None and name_elem.text == layer_name:
                return layer
        return None

    def _layer_exists(self, xml_result, layer_name):
        """Check if a layer with the given name exists in the XML response"""
        return self._get_layer(xml_result, layer_name) is not None

    def _layer_attribute(self, xml_result, layer_name, attribute_name):
        """Get a layer attribute value. Returns None if layer or attribute not found."""
        layer = self._get_layer(xml_result, layer_name)
        return layer.get(attribute_name)

    def _all_children_exists(self, xml_result, parent_name, children_names):
        """Check if a layer with the given name and children exists in the XML response"""
        parent_layer = self._get_layer(xml_result, parent_name)

        found_children = []
        for child_layer in parent_layer.findall("Layer"):
            child_name_elem = child_layer.find("Name")
            if child_name_elem is not None:
                found_children.append(child_name_elem.text)

        return set(children_names) == set(found_children)

    def testGetCapabilities(self):
        """
        Test getcapabilities response.

        We will find there
        - street-group as group (with layer objects)
        - streetline as layer
        - streetarea as layer
        - road-opaquegroup as layer (without any layer objects)
        - way-opaquegroup-excludedparts (without any layer objects)
        - sign-opaquegroup-samenamed (without any layer objects)
        - trafficsignpoint as layer
        We will not find there
        - roadline
        - roadarea
        - wayline
        - wayarea-excluded
        """

        (_, body, request) = self.wms_request("GetCapabilities", project=self.project)

        xmlResult = body.decode("utf-8")

        # Check for specific layer structures
        self.assertTrue(
            self._layer_exists(xmlResult, "Roads"),
            f"Layer 'Roads' not found in response",
        )
        self.assertTrue(
            self._all_children_exists(xmlResult, "Roads", ["Road1", "Road2"])
        )
        self.assertEqual(self._layer_attribute(xmlResult, "Roads", "opaque"), 0)

    def testGetContext(self):
        """
        Test getcontext response.

        We will find there
        - ... let's see...
        """
        (_, body, request) = self.wms_request("GetContext", project=self.project)

    def testGetProjectSettings(self):
        """
        Test getprojectsettings response.

        We will find there
        - ... let's see...
        """

        (_, body, request) = self.wms_request(
            "GetProjectSettings", project=self.project
        )

    def testGetFeatureInfo(self):
        """
        Test getfeatureinfo response.

        We will find there
        - ... let's see...
        """

        self.wms_request(
            "GetFeatureInfo",
            reference_file="wms-opaquegroups-getfeatureinfo-map",
            project=self.project,
        )

        self.wms_request(
            "GetFeatureInfo",
            "&layers=testlayer%20%C3%A8%C3%A9&styles=&"
            + "info_format=text%2Fxml&transparent=true&"
            + "width=600&height=400&srs=EPSG%3A3857&bbox=913190.6389747962%2C"
            + "5606005.488876367%2C913235.426296057%2C5606035.347090538&"
            + "query_layers=testlayer%20%C3%A8%C3%A9&X=190&Y=320",
            "wms_getfeatureinfo-text-xml",
        )


if __name__ == "__main__":
    unittest.main()
