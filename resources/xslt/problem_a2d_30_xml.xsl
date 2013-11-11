<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:problem="XMLProblem"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

<xsl:output method="xml" version="1.0" encoding="utf-8"
            omit-xml-declaration="yes"/>

<xsl:template match="/">
    <xsl:element name="problem:document">
        <xsl:attribute name="version">3</xsl:attribute>

        <!-- geometry -->
        <xsl:element name="geometry">
            <!-- nodes -->
            <xsl:copy-of select="/document/geometry/nodes"/>

            <!-- edges -->
            <xsl:element name="edges">
                <xsl:for-each select="/document/geometry/edges/edge">
                    <xsl:element name="edge">
                        <xsl:attribute name="id">
                            <xsl:value-of select ="@id"/>
                        </xsl:attribute>
                        <xsl:attribute name="start">
                          <xsl:value-of select ="@start"/>
                        </xsl:attribute>
                        <xsl:attribute name="end">
                          <xsl:value-of select ="@end"/>
                        </xsl:attribute>
                        <xsl:attribute name="angle">
                          <xsl:value-of select ="@angle"/>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>

            <!-- edges -->
            <xsl:element name="labels">
                <xsl:for-each select="/document/geometry/labels/label">
                    <xsl:element name="label">
                        <xsl:attribute name="id">
                            <xsl:value-of select ="@id"/>
                        </xsl:attribute>
                        <xsl:attribute name="x">
                          <xsl:value-of select ="@x"/>
                        </xsl:attribute>
                        <xsl:attribute name="y">
                          <xsl:value-of select ="@y"/>
                        </xsl:attribute>
                        <xsl:attribute name="area">
                          <xsl:value-of select ="@area"/>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
        </xsl:element>

        <!-- problem -->
        <xsl:element name="problem">
            <xsl:attribute name="coordinate_type">
              <xsl:value-of select ="/document/problem/@coordinate_type"/>
            </xsl:attribute>
            <xsl:attribute name="matrix_solver">
              <xsl:value-of select ="/document/problem/@matrix_solver"/>
            </xsl:attribute>
            <xsl:attribute name="mesh_type">
                <xsl:choose>
                    <xsl:when test="/document/problem/@mesh_type">
                        <xsl:value-of select="/document/problem/@mesh_type" />
                    </xsl:when>
                    <xsl:otherwise>triangle</xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:attribute name="frequency">
                <xsl:value-of select="/document/problem/@frequency" />
            </xsl:attribute>
            <xsl:attribute name="time_total">
                <xsl:value-of select="/document/problem/@time_total" />
            </xsl:attribute>
            <xsl:attribute name="time_steps">
                <xsl:value-of select="/document/problem/@time_steps" />
            </xsl:attribute>

            <!-- fields -->
            <xsl:element name="fields">
                <xsl:for-each select="/document/problem/fields/field">
                    <xsl:element name="field">
                        <xsl:attribute name="field_id">
                          <xsl:value-of select ="@field_id"/>
                        </xsl:attribute>
                        <xsl:attribute name="analysis_type">
                          <xsl:value-of select ="@analysis_type"/>
                        </xsl:attribute>
                        <xsl:attribute name="initial_condition">
                          <xsl:value-of select ="@initial_condition"/>
                        </xsl:attribute>
                        <xsl:attribute name="polynomial_order">
                          <xsl:value-of select ="@polynomial_order"/>
                        </xsl:attribute>
                        <xsl:attribute name="number_of_refinements">
                          <xsl:value-of select ="@number_of_refinements"/>
                        </xsl:attribute>
                        <xsl:attribute name="time_skip">
                          <xsl:value-of select ="@time_skip"/>
                        </xsl:attribute>

                        <!-- edges refinement -->
                        <xsl:element name="refinement_edges">
                            <xsl:for-each select="refinement/edges/edge">
                            <xsl:element name="refinement_edge">
                                <xsl:attribute name="refinement_edge_id">
                                  <xsl:value-of select ="@edge"/>
                                </xsl:attribute>
                                <xsl:attribute name="refinement_edge_number">
                                  <xsl:value-of select ="@refinement"/>
                                </xsl:attribute>
                            </xsl:element>
                            </xsl:for-each>
                        </xsl:element>

                        <!-- labels refinement -->
                        <xsl:element name="refinement_labels">
                            <xsl:for-each select="refinement/labels/label">
                            <xsl:element name="refinement_edges">
                                <xsl:attribute name="refinement_label_id">
                                  <xsl:value-of select ="@label"/>
                                </xsl:attribute>
                                <xsl:attribute name="refinement_label_number">
                                  <xsl:value-of select ="@refinement"/>
                                </xsl:attribute>
                            </xsl:element>
                            </xsl:for-each>
                        </xsl:element>

                        <!-- polynomial orders -->
                        <xsl:element name="polynomial_orders">
                            <xsl:for-each select="polynomial_order/label">
                            <xsl:element name="polynomial_order">
                                <xsl:attribute name="polynomial_order_id">
                                  <xsl:value-of select ="@label"/>
                                </xsl:attribute>
                                <xsl:attribute name="polynomial_order_number">
                                  <xsl:value-of select ="@order"/>
                                </xsl:attribute>
                            </xsl:element>
                            </xsl:for-each>
                        </xsl:element>

                        <xsl:copy-of select="adaptivity"/>
                        <xsl:copy-of select="solver"/>

                        <!-- boundaries -->
                        <xsl:element name="boundaries">
                            <xsl:for-each select="boundaries/boundary">
                                <xsl:element name="boundary">
                                    <xsl:attribute name="id">
                                      <xsl:value-of select ="@id"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="name">
                                      <xsl:value-of select ="@name"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="type">
                                      <xsl:value-of select ="@type"/>
                                    </xsl:attribute>

                                    <!-- edges -->
                                    <xsl:element name="boundary_edges">
                                        <xsl:for-each select="edge">
                                            <xsl:element name="boundary_edge">
                                                <xsl:attribute name="id">
                                                  <xsl:value-of select ="@edge"/>
                                                </xsl:attribute>
                                            </xsl:element>
                                        </xsl:for-each>
                                    </xsl:element>

                                    <!-- types -->
                                    <xsl:element name="boundary_types">
                                        <xsl:for-each select="@*">
                                            <xsl:if test="not(name(.)='name') and not(name(.)='id') and not(name(.)='type')">
                                                <xsl:element name="boundary_type">
                                                    <xsl:attribute name="key">
                                                        <xsl:value-of select ="name(.)"/>
                                                    </xsl:attribute>
                                                    <xsl:attribute name="value">
                                                        <xsl:value-of select="." />
                                                    </xsl:attribute>
                                                </xsl:element>
                                            </xsl:if>
                                        </xsl:for-each>
                                    </xsl:element>
                                </xsl:element>
                            </xsl:for-each>
                        </xsl:element>

                        <!-- materials -->
                        <xsl:element name="materials">
                            <xsl:for-each select="materials/material">
                                <xsl:element name="material">
                                    <xsl:attribute name="id">
                                      <xsl:value-of select ="@id"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="name">
                                      <xsl:value-of select ="@name"/>
                                    </xsl:attribute>

                                    <!-- label -->
                                    <xsl:element name="material_labels">
                                        <xsl:for-each select="label">
                                            <xsl:element name="material_label">
                                                <xsl:attribute name="id">
                                                  <xsl:value-of select ="@label"/>
                                                </xsl:attribute>
                                            </xsl:element>
                                        </xsl:for-each>
                                    </xsl:element>

                                    <!-- types -->
                                    <xsl:element name="material_types">
                                        <xsl:for-each select="@*">
                                            <xsl:if test="not(name(.)='name') and not(name(.)='id') and not(name(.)='type')">
                                                <xsl:element name="material_type">
                                                    <xsl:attribute name="key">
                                                        <xsl:value-of select ="name(.)"/>
                                                    </xsl:attribute>
                                                    <xsl:attribute name="value">
                                                        <xsl:value-of select="." />
                                                    </xsl:attribute>
                                                </xsl:element>
                                            </xsl:if>
                                        </xsl:for-each>
                                    </xsl:element>
                                </xsl:element>
                            </xsl:for-each>
                        </xsl:element>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>

            <!-- couplings -->
            <xsl:choose>
                <xsl:when test="/document/problem/couplings">
                    <xsl:copy-of select="/document/problem/couplings"/>
                </xsl:when>
                <xsl:otherwise><xsl:element name="couplings"></xsl:element></xsl:otherwise>
            </xsl:choose>
        </xsl:element>

        <!-- config -->
        <xsl:element name="config">
            <xsl:variable name="linebreak">
                <xsl:text>
</xsl:text>
            </xsl:variable>
            <xsl:for-each select="/document/config/@*">
                <xsl:element name="item">
                    <xsl:attribute name="key">
                        <xsl:value-of select ="name(.)"/>
                    </xsl:attribute>
                    <xsl:attribute name="value">
                        <xsl:value-of select="translate(., '&amp;#xA;', ';')" />
                    </xsl:attribute>
                </xsl:element>
            </xsl:for-each>
        </xsl:element>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
