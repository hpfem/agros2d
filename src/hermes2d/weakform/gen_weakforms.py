#!/usr/bin/python

from xml_parser import XmlParser

modules = []
# modules = ['electrostatic', 'current', 'heat', 'acoustic', 'elasticity', 'rf', 'magnetic']
# modules = ['magnetic']
xml_parser = XmlParser(modules)
xml_parser.process()
xml_parser.gen_doc()
