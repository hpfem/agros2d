#!/usr/bin/python

from xml_parser import XmlParser

modules = ['electrostatic', 'heat', 'current']
xml_parser = XmlParser(modules)
xml_parser.process()
