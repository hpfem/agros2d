# -*- coding: utf-8 -*-
#!/usr/bin/python
"""
Created on Wed Aug 31 09:20:37 2011

@author: david
"""

from xml_parser import XmlParser

modules = ['electrostatic', 'current', 'heat']
xml_parser = XmlParser(modules)
xml_parser.process()
