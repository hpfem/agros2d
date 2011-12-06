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
                        <xsl:choose>
                            <!-- Transformation from version 2.0 -->
                            <xsl:when test="$current_version!=null">
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
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- Transformation from version 1.0 -->
                                <xsl:variable name="edges" select="document/geometry/edges/*" />
                                <xsl:for-each select="document/problems/problem/edges/edge">
                                    <xsl:element name="boundary">
                                        <!-- Magnetic field -->
                                        <xsl:if test="@type='magnetic_vector_potential'">
                                            <xsl:attribute name="type">magnetic_potential</xsl:attribute>
                                            <xsl:attribute name="magnetic_potential_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="magnetic_potential_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='magnetic_surface_current_density'">
                                            <xsl:attribute name="type">magnetic_surface_current</xsl:attribute>
                                            <xsl:attribute name="magnetic_surface_current_imag">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="magnetic_surface_current_real">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- Electrostatic field -->
                                        <xsl:if test="@type='electrostatic_potential'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="electrostatic_potential">
                                                <xsl:value-of select="@value"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='electrostatic_surface_charge_density'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="electrostatic_surface_charge_density">
                                                <xsl:value-of select="@value"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- Temperature field -->
                                        <xsl:if test="@type='heat_temperature'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="heat_temperature">
                                                <xsl:value-of select="@temperature"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='heat_heat_flux'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="heat_heat_flux">
                                                <xsl:value-of select="@heat_flux"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='heat_heat_flux'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="heat_heat_transfer_coefficient">
                                                <xsl:value-of select="@h"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='heat_heat_flux'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="external_temperature">
                                                <xsl:value-of select="@heat_external_temperature"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- Acustic field -->
                                        <xsl:if test="@type='acoustic_pressure'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="acoustic_pressure">
                                                <xsl:value-of select="@pressure"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='acoustic_normal_acceleration'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="acoustic_normal_acceleration">
                                                <xsl:value-of select="@acceleration"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='acoustic_impedance'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="acoustic_impedance">
                                                <xsl:value-of select="@acoustic_impedance"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- Current field -->
                                        <xsl:if test="@type='current_potential'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="current_potential">
                                                <xsl:value-of select="@value"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- RF field -->
                                        <xsl:if test="@type='rf_electric_field'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_electric_field_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_electric_field_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_magnetic_field'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_magnetic_field_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_magnetic_field_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_surface_current'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_surface_current_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_surface_current_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_port'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_port_power">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_port_phase">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <xsl:attribute name="id">
                                            <xsl:value-of select="@id"/>
                                        </xsl:attribute>
                                        <xsl:attribute name="name">
                                            <xsl:value-of select="@name"/>
                                        </xsl:attribute>

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
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>

                    <!-- Materials -->
                    <xsl:element name="materials">
                        <xsl:choose>
                            <!-- Transformation from version 2.0 -->
                            <xsl:when test="$current_version!=null">
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
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- Transformation from version 1.0 -->
                                <xsl:variable name="labels" select="document/geometry/labels/*" />
                                <xsl:variable name="physical_field" select="document/problems/problem/@type" />
                                <xsl:for-each select="document/problems/problem/labels/label">
                                    <xsl:element name="material">
                                        <!-- Magnetic field -->
                                        <xsl:if test="$physical_field='magnetic'">
                                            <xsl:if test="@current_density_real">
                                                <xsl:attribute name="magnetic_current_density_external_real">
                                                    <xsl:value-of select="@current_density_real"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@remanence">
                                                <xsl:attribute name="magnetic_remanence">
                                                    <xsl:value-of select="@remanence"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@conductivity">
                                                <xsl:attribute name="magnetic_conductivity">
                                                    <xsl:value-of select="@conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@velocity_x">
                                                <xsl:attribute name="magnetic_velocity_x">
                                                    <xsl:value-of select="@velocity_x"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@velocity_y">
                                                <xsl:attribute name="magnetic_velocity_y">
                                                    <xsl:value-of select="@velocity_y"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@remanence_angle">
                                                <xsl:attribute name="magnetic_remanence_angle">
                                                    <xsl:value-of select="@remanence_angle"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@current_density_imag">
                                                <xsl:attribute name="magnetic_current_density_external_imag">
                                                    <xsl:value-of select="@current_density_imag"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@permeability">
                                                <xsl:attribute name="magnetic_permeability">
                                                    <xsl:value-of select="@permeability"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@velocity_angular">
                                                <xsl:attribute name="magnetic_velocity_angular">
                                                    <xsl:value-of select="@velocity_angular"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- Electrostatic field -->
                                        <xsl:if test="$physical_field='electrostatic'">
                                            <xsl:if test="@permittivity">
                                                <xsl:attribute name="electrostatic_permittivity">
                                                    <xsl:value-of select="@permittivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@charge_density">
                                                <xsl:attribute name="electrostatic_charge_density">
                                                    <xsl:value-of select="@charge_density"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- Temperature field -->
                                        <xsl:if test="$physical_field='heat'">
                                            <xsl:if test="@thermal_conductivity">
                                                <xsl:attribute name="heat_conductivity">
                                                    <xsl:value-of select="@thermal_conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@volume_heat">
                                                <xsl:attribute name="heat_volume_heat">
                                                    <xsl:value-of select="@volume_heat"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@density">
                                                <xsl:attribute name="heat_density">
                                                    <xsl:value-of select="@density"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@specific_heat">
                                                <xsl:attribute name="heat_specific_heat">
                                                    <xsl:value-of select="@specific_heat"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- Acoustic field -->
                                        <xsl:if test="$physical_field='acoustic'">
                                            <xsl:if test="@density">
                                                <xsl:attribute name="acoustic_density">
                                                    <xsl:value-of select="@density"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@speed">
                                                <xsl:attribute name="acoustic_speed">
                                                    <xsl:value-of select="@speed"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- Current field -->
                                        <xsl:if test="$physical_field='current'">
                                            <xsl:if test="@conductivity">
                                                <xsl:attribute name="current_conductivity">
                                                    <xsl:value-of select="@conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- RF field -->
                                        <xsl:if test="$physical_field='rf'">
                                            <xsl:if test="@permittivity">
                                                <xsl:attribute name="rf_permittivity">
                                                    <xsl:value-of select="@permittivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@permeability">
                                                <xsl:attribute name="rf_permeability">
                                                    <xsl:value-of select="@permittivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@conductivity">
                                                <xsl:attribute name="rf_conductivity">
                                                    <xsl:value-of select="@conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@current_density_real">
                                                <xsl:attribute name="rf_current_density_external_real">
                                                    <xsl:value-of select="@current_density_real"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@current_density_imag">
                                                <xsl:attribute name="rf_current_density_external_imag">
                                                    <xsl:value-of select="@current_density_imag"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <xsl:attribute name="id">
                                            <xsl:value-of select="@id"/>
                                        </xsl:attribute>
                                        <xsl:attribute name="name">
                                            <xsl:value-of select="@name"/>
                                        </xsl:attribute>

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
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>
                </xsl:element>
            </xsl:element>
        </xsl:element>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
