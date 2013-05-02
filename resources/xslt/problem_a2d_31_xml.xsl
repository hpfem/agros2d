<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:problem="XMLProblem"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

<xsl:output method="xml" version="1.0" encoding="utf-8"
            omit-xml-declaration="yes"/>

<xsl:template match="/">
    <xsl:element name="problem:document">
        <xsl:attribute name="version">3.1</xsl:attribute>

        <!-- geometry -->
        <xsl:copy-of select="problem:document/geometry"/>

        <!-- problem -->
        <xsl:element name="problem">
            <xsl:attribute name="coordinate_type">
              <xsl:value-of select ="problem:document/problem/@coordinate_type"/>
            </xsl:attribute>
            <xsl:attribute name="matrix_solver">
              <xsl:value-of select ="problem:document/problem/@matrix_solver"/>
            </xsl:attribute>
            <xsl:attribute name="mesh_type">
              <xsl:value-of select ="problem:document/problem/@mesh_type"/>
            </xsl:attribute>

            <!-- fields -->
            <xsl:element name="fields">
                <xsl:for-each select="problem:document/problem/fields/field">
                    <xsl:element name="field">
                        <xsl:attribute name="field_id">
                          <xsl:value-of select ="@field_id"/>
                        </xsl:attribute>
                        <xsl:attribute name="adaptivity_type">
                          <xsl:value-of select ="adaptivity/@adaptivity_type"/>
                        </xsl:attribute>
                        <xsl:attribute name="analysis_type">
                          <xsl:value-of select ="@analysis_type"/>
                        </xsl:attribute>
                        <xsl:attribute name="linearity_type">
                          <xsl:value-of select ="solver/@linearity_type"/>
                        </xsl:attribute>

                        <xsl:copy-of select="refinement_edges"/>
                        <xsl:copy-of select="refinement_labels"/>
                        <xsl:copy-of select="polynomial_orders"/>
                        <xsl:copy-of select="boundaries"/>
                        <xsl:copy-of select="materials"/>

                        <!-- field config -->
                        <xsl:element name="field_config">
                            <xsl:for-each select="@*">
                                <xsl:element name="field_item">
                                    <xsl:attribute name="field_key">
                                        <xsl:value-of select ="name(.)"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="field_value">
                                        <xsl:value-of select="." />
                                    </xsl:attribute>
                                </xsl:element>
                            </xsl:for-each>

                            <!-- adaptivity config -->
                            <xsl:for-each select="./adaptivity/@*">
                                <xsl:element name="field_item">
                                    <xsl:attribute name="field_key">
                                        <xsl:value-of select ="name(.)"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="field_value">
                                        <xsl:value-of select="." />
                                    </xsl:attribute>
                                </xsl:element>
                            </xsl:for-each>

                            <!-- solver config -->
                            <xsl:for-each select="./solver/@*">
                                <xsl:element name="field_item">
                                    <xsl:attribute name="field_key">
                                        <xsl:value-of select ="name(.)"/>
                                    </xsl:attribute>
                                    <xsl:attribute name="field_value">
                                        <xsl:value-of select="." />
                                    </xsl:attribute>
                                </xsl:element>
                            </xsl:for-each>
                        </xsl:element>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>

            <xsl:copy-of select="problem:document/problem/couplings"/>

            <!-- problem config -->
            <xsl:element name="problem_config">
                <xsl:for-each select="problem:document/problem/@*">
                    <xsl:element name="problem_item">
                        <xsl:attribute name="problem_key">
                            <xsl:value-of select ="name(.)"/>
                        </xsl:attribute>
                        <xsl:attribute name="problem_value">
                            <xsl:value-of select="." />
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>

            <!-- couplings -->
            <xsl:copy-of select="couplings"/>
        </xsl:element>

        <!-- config -->
        <xsl:copy-of select="problem:document/config"/>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
