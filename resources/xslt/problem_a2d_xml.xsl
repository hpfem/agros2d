<?xml version="1.0"?>
<xsl:stylesheet version="2.1" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
    <xsl:element name="document">
        <xsl:attribute name="version">2.1</xsl:attribute>
        <xsl:variable name="current_version" select="document/@version" />

        <!-- Geometry -->
        <xsl:element name="geometry">
            <xsl:copy-of select="document/geometry/nodes"/>
            <xsl:element name="edges">
                <xsl:for-each select="document/geometry/edges/edge">
                    <xsl:element name="edge">
                        <xsl:attribute name="id">
                            <xsl:value-of select="@id" />
                        </xsl:attribute>
                        <xsl:attribute name="start">
                            <xsl:value-of select="@start" />
                        </xsl:attribute>
                        <xsl:attribute name="end">
                            <xsl:value-of select="@end" />
                        </xsl:attribute>
                        <xsl:attribute name="angle">
                            <xsl:value-of select="@angle" />
                        </xsl:attribute>
                        <xsl:attribute name="refine_towards">
                            <xsl:value-of select="@refine_towards" />
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
            <xsl:element name="labels">
                <xsl:for-each select="document/geometry/labels/label">
                    <xsl:element name="label">
                        <xsl:attribute name="id">
                            <xsl:value-of select="@id" />
                        </xsl:attribute>
                        <xsl:attribute name="x">
                            <xsl:value-of select="@x" />
                        </xsl:attribute>
                        <xsl:attribute name="y">
                            <xsl:value-of select="@y" />
                        </xsl:attribute>
                        <xsl:attribute name="area">
                            <xsl:value-of select="@area" />
                        </xsl:attribute>
                        <xsl:attribute name="polynomialorder">
                            <xsl:value-of select="@polynomialorder" />
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
        </xsl:element>

        <!-- Problem -->
        <xsl:element name="problem">
            <xsl:attribute name="coordinatetype">
                <xsl:value-of select="document/problems/problem/@problemtype" />
            </xsl:attribute>
            <xsl:attribute name="frequency">
                <xsl:value-of select="document/problems/problem/@frequency" />
            </xsl:attribute>
            <xsl:attribute name="matrix_solver">
                <xsl:value-of select="document/problems/problem/@matrix_solver" />
            </xsl:attribute>
            <xsl:attribute name="timetotal">
                <xsl:value-of select="document/problems/problem/@timetotal" />
            </xsl:attribute>
            <xsl:attribute name="type">
                <xsl:value-of select="document/problems/problem/@meshtype" />
            </xsl:attribute>
            <xsl:attribute name="timestep">
                <xsl:value-of select="document/problems/problem/@timestep" />
            </xsl:attribute>
            <xsl:attribute name="name">
                <xsl:value-of select="document/problems/problem/@name" />
            </xsl:attribute>
            <xsl:attribute name="date">
                <xsl:value-of select="document/problems/problem/@date" />
            </xsl:attribute>

            <xsl:element name="description">
                <xsl:value-of select="document/problems/problem/description" />
            </xsl:element>

            <xsl:element name="startupscript">
                <xsl:value-of select="document/problems/problem/scriptstartup" />
            </xsl:element>

            <xsl:element name="fields">
                <xsl:element name="field">
                    <xsl:attribute name="fieldid">
                        <xsl:value-of select="document/problems/problem/@type" />
                    </xsl:attribute>
                    <xsl:attribute name="numberofrefinements">
                        <xsl:value-of select="document/problems/problem/@numberofrefinements" />
                    </xsl:attribute>
                    <xsl:attribute name="analysistype">
                        <xsl:value-of select="document/problems/problem/@analysistype" />
                    </xsl:attribute>
                    <xsl:attribute name="polynomialorder">
                        <xsl:value-of select="document/problems/problem/@polynomialorder" />
                    </xsl:attribute>
                    <xsl:attribute name="weakforms">
                        <xsl:variable name="weakforms" select="document/problems/problem/@weakforms" />
                        <xsl:choose>
                            <xsl:when test="$weakforms!=null">
                                <xsl:value-of select="$weakforms" />
                            </xsl:when>
                            <xsl:otherwise>interpreted</xsl:otherwise>
                        </xsl:choose>
                    </xsl:attribute>
                    <xsl:attribute name="initialcondition">
                        <xsl:value-of select="document/problems/problem/@initialcondition" />
                    </xsl:attribute>

                    <xsl:element name="adaptivity">
                        <xsl:attribute name="adaptivitytolerance">
                            <xsl:value-of select="document/problems/problem/@adaptivitytolerance" />
                        </xsl:attribute>
                        <xsl:attribute name="adaptivitytype">
                            <xsl:value-of select="document/problems/problem/@adaptivitytype" />
                        </xsl:attribute>
                        <xsl:attribute name="adaptivitysteps">
                            <xsl:value-of select="document/problems/problem/@adaptivitysteps" />
                        </xsl:attribute>
                        <xsl:attribute name="maxdofs">
                            <xsl:value-of select="document/problems/problem/@maxdofs" />
                        </xsl:attribute>
                    </xsl:element>

                    <xsl:element name="solver">
                        <xsl:attribute name="nonlineartolerance">
                            <xsl:variable name="nonlineartolerance" select="document/problems/problem/@nonlineartolerance" />
                            <xsl:choose>
                                <xsl:when test="$nonlineartolerance!=null">
                                    <xsl:value-of select="$nonlineartolerance" />
                                </xsl:when>
                                <xsl:otherwise>interpreted</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                        <xsl:attribute name="nonlinearsteps">
                            <xsl:variable name="nonlinearsteps" select="document/problems/problem/@nonlinearsteps" />
                            <xsl:choose>
                                <xsl:when test="$nonlinearsteps!=null">
                                    <xsl:value-of select="$nonlinearsteps" />
                                </xsl:when>
                                <xsl:otherwise>interpreted</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                        <xsl:attribute name="nonlinearsteps">
                            <xsl:variable name="linearity" select="document/problems/problem/@linearity" />
                            <xsl:choose>
                                <xsl:when test="$linearity!=null">
                                    <xsl:value-of select="$linearity" />
                                </xsl:when>
                                <xsl:otherwise>interpreted</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                    </xsl:element>

                    <!-- Boundaries -->
                    <xsl:element name="boundaries">
                        <xsl:variable name="edges" select="document/geometry/edges/*" />
                        <xsl:for-each select="document/problems/problem/edges/edge">
                            <xsl:element name="boundary">
                                <xsl:for-each select="@*">
                                    <xsl:attribute name="{name()}">
                                        <xsl:value-of select="."/>
                                    </xsl:attribute>
                                </xsl:for-each>
                                <xsl:variable name="id" select="@id" />
                                <xsl:for-each select="$edges">
                                    <xsl:if test="@marker=$id">
                                        <xsl:element name="edge">
                                            <xsl:attribute name="edge">
                                                <xsl:value-of select="@id" />
                                            </xsl:attribute>
                                        </xsl:element>
                                    </xsl:if>
                                </xsl:for-each>
                            </xsl:element>
                        </xsl:for-each>
                    </xsl:element>

                    <!-- Materials -->
                    <xsl:element name="materials">
                        <xsl:variable name="labels" select="document/geometry/labels/*" />
                        <xsl:for-each select="document/problems/problem/labels/label">
                            <xsl:element name="material">
                                <xsl:for-each select="@*">
                                    <xsl:attribute name="{name()}">
                                        <xsl:value-of select="."/>
                                    </xsl:attribute>
                                </xsl:for-each>
                                <xsl:variable name="id" select="@id" />
                                <xsl:for-each select="$labels">
                                    <xsl:if test="@marker=$id">
                                        <xsl:element name="label">
                                            <xsl:attribute name="label">
                                                <xsl:value-of select="@id" />
                                            </xsl:attribute>
                                        </xsl:element>
                                    </xsl:if>
                                </xsl:for-each>
                            </xsl:element>
                        </xsl:for-each>
                    </xsl:element>
                </xsl:element>
            </xsl:element>
        </xsl:element>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
