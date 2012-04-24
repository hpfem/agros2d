#!/usr/bin/python

from xml_parser import XmlParser

modules = ['electrostatic', 'current', 'heat', 'acoustic', 'elasticity', 'rf', 'magnetic']
# modules = ['electrostatic', 'current', 'heat', 'acoustic', 'elasticity', 'magnetic']
xml_parser = XmlParser(modules)
xml_parser.process()
#xml_parser.gen_doc()
