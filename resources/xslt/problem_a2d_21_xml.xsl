<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" version="1.0" encoding="utf-8" />

<xsl:template name="edges">
    <xsl:param name="id" />
    <xsl:for-each select="/document/geometry/edges/edge[@marker=$id]">
        <xsl:element name="edge">
            <xsl:attribute name="edge">
                <xsl:value-of select="@id" />
            </xsl:attribute>
        </xsl:element>
    </xsl:for-each>
</xsl:template>

<xsl:template name="labels">
    <xsl:param name="id" />
    <xsl:for-each select="/document/geometry/labels/label[@marker=$id]">
        <xsl:element name="label">
            <xsl:attribute name="label">
                <xsl:value-of select="@id" />
            </xsl:attribute>
        </xsl:element>
    </xsl:for-each>
</xsl:template>

<xsl:template match="/">
    <xsl:element name="document">
        <xsl:attribute name="version">2.1</xsl:attribute>
        <xsl:variable name="current_version" select="/document/@version" />

        <!-- geometry -->
        <xsl:element name="geometry">
            <xsl:copy-of select="/document/geometry/nodes"/>
            <xsl:element name="edges">
                <xsl:for-each select="/document/geometry/edges/edge">
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
                            <xsl:variable name="refine_towards" select="/document/geometry/edges/edge/@refine_towards" />
                            <xsl:choose>
                                <xsl:when test="$refine_towards">
                                    <xsl:value-of select="$refine_towards" />
                                </xsl:when>
                                <xsl:otherwise>0</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
            <xsl:element name="labels">
                <xsl:for-each select="/document/geometry/labels/label">
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
                        <xsl:attribute name="polynomial_order">
                            <xsl:variable name="polynomial_order" select="/document/geometry/labels/label/@polynomial_order" />
                            <xsl:choose>
                                <xsl:when test="$polynomial_order">
                                    <xsl:value-of select="$polynomial_order" />
                                </xsl:when>
                                <xsl:otherwise>0</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
        </xsl:element>

        <!-- problem -->
        <xsl:element name="problem">
            <xsl:attribute name="coordinate_type">
                <xsl:value-of select="/document/problems/problem/@problemtype" />
            </xsl:attribute>
            <xsl:attribute name="matrix_solver">
                <xsl:value-of select="/document/problems/problem/@matrix_solver" />
            </xsl:attribute>
            <xsl:attribute name="mesh_type">
                <xsl:value-of select="/document/problems/problem/@meshtype" />
            </xsl:attribute>
            <xsl:attribute name="date">
                <xsl:value-of select="/document/problems/problem/@date" />
            </xsl:attribute>

            <!-- transient problem -->
            <xsl:if test="/document/problems/problem/@analysistype='transient'">
                <xsl:variable name="time_total" select="/document/problems/problem/@timetotal" />
                <xsl:variable name="time_step" select="/document/problems/problem/@timestep" />
                <xsl:attribute name="time_total">
                    <xsl:value-of select="$time_total" />
                </xsl:attribute>
                <xsl:attribute name="time_steps">
                    <xsl:value-of select="number($time_total div $time_step)" />
                </xsl:attribute>
            </xsl:if>

            <!-- harmonic problem -->
            <xsl:if test="/document/problems/problem/@analysistype='harmonic'">
                <xsl:attribute name="frequency">
                    <xsl:value-of select="/document/problems/problem/@frequency" />
                </xsl:attribute>
            </xsl:if>

            <xsl:element name="description">
                <xsl:value-of select="/document/problems/problem/description" />
            </xsl:element>

            <!-- fields -->
            <xsl:element name="fields">
                <xsl:element name="field">
                    <xsl:attribute name="field_id">
                        <xsl:value-of select="/document/problems/problem/@type" />
                    </xsl:attribute>
                    <xsl:attribute name="number_of_refinements">
                        <xsl:value-of select="/document/problems/problem/@numberofrefinements" />
                    </xsl:attribute>
                    <xsl:attribute name="analysis_type">
                        <xsl:value-of select="/document/problems/problem/@analysistype" />
                    </xsl:attribute>
                    <xsl:attribute name="polynomial_order">
                        <xsl:value-of select="/document/problems/problem/@polynomialorder" />
                    </xsl:attribute>
                    <xsl:attribute name="initial_condition">
                        <xsl:value-of select="/document/problems/problem/@initialcondition" />
                    </xsl:attribute>

                    <xsl:element name="adaptivity">
                        <xsl:attribute name="adaptivity_tolerance">
                            <xsl:value-of select="/document/problems/problem/@adaptivitytolerance" />
                        </xsl:attribute>
                        <xsl:attribute name="adaptivity_type">
                            <xsl:value-of select="/document/problems/problem/@adaptivitytype" />
                        </xsl:attribute>
                        <xsl:attribute name="adaptivity_steps">
                            <xsl:value-of select="/document/problems/problem/@adaptivitysteps" />
                        </xsl:attribute>
                        <xsl:attribute name="max_dofs">
                            <xsl:variable name="max_dofs" select="/document/problems/problem/@maxdofs" />
                            <xsl:choose>
                                <xsl:when test="$max_dofs">
                                    <xsl:value-of select="$max_dofs" />
                                </xsl:when>
                                <xsl:otherwise>60000</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                    </xsl:element>

                    <xsl:element name="solver">
                        <xsl:attribute name="nonlinear_tolerance">
                            <xsl:variable name="nonlinear_tolerance" select="/document/problems/problem/@nonlineartolerance" />
                            <xsl:choose>
                                <xsl:when test="$nonlinear_tolerance">
                                    <xsl:value-of select="$nonlinear_tolerance" />
                                </xsl:when>
                                <xsl:otherwise>1e-3</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                        <xsl:attribute name="nonlinear_steps">
                            <xsl:variable name="nonlinear_steps" select="/document/problems/problem/@nonlinearsteps" />
                            <xsl:choose>
                                <xsl:when test="$nonlinear_steps">
                                    <xsl:value-of select="$nonlinear_steps" />
                                </xsl:when>
                                <xsl:otherwise>10</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                        <xsl:attribute name="linearity_type">
                            <xsl:variable name="linearity_type" select="/document/problems/problem/@linearity" />
                            <xsl:choose>
                                <xsl:when test="$linearity_type">
                                    <xsl:value-of select="$linearity_type" />
                                </xsl:when>
                                <xsl:otherwise>linear</xsl:otherwise>
                            </xsl:choose>
                        </xsl:attribute>
                    </xsl:element>

                    <!-- boundaries -->
                    <xsl:element name="boundaries">
                        <xsl:choose>
                            <xsl:when test="$current_version">
                                <!-- transformation from version 2.0 -->
                                <xsl:for-each select="/document/problems/problem/edges/edge">
                                    <xsl:element name="boundary">
                                        <xsl:for-each select="@*">
                                            <xsl:attribute name="{name()}">
                                                <xsl:value-of select="."/>
                                            </xsl:attribute>
                                        </xsl:for-each>
                                        <xsl:call-template name="edges">
                                            <xsl:with-param name="id" select="@id" />
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- transformation from version 1.0 -->
                                <xsl:for-each select="/document/problems/problem/edges/edge">
                                    <xsl:element name="boundary">
                                        <!-- magnetic field -->
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

                                        <!-- electrostatic field -->
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

                                        <!-- temperature field -->
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
                                            <xsl:attribute name="heat_convection_heat_transfer_coefficient">
                                                <xsl:value-of select="@h"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="heat_convection_external_temperature">
                                                <xsl:value-of select="@external_temperature"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- acustic field -->
                                        <xsl:if test="@type='acoustic_pressure'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="acoustic_pressure_real">
                                                <xsl:value-of select="@pressure"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="acoustic_pressure_imag">0</xsl:attribute>
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
                                        <xsl:if test="@type='acoustic_matched_boundary'">
                                            <xsl:attribute name="type">acoustic_impedance</xsl:attribute>
                                            <xsl:attribute name="acoustic_impedance">428.75</xsl:attribute>
                                        </xsl:if>

                                        <!-- current field -->
                                        <xsl:if test="@type='current_potential'">
                                            <xsl:attribute name="type">
                                                <xsl:value-of select="@type"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="current_potential">
                                                <xsl:value-of select="@value"/>
                                            </xsl:attribute>
                                        </xsl:if>

                                        <!-- rf field -->
                                        <xsl:if test="@type='rf_electric_field'">
                                            <xsl:attribute name="type">rf_te_electric_field</xsl:attribute>
                                            <xsl:attribute name="rf_te_electric_field_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_te_electric_field_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_magnetic_field'">
                                            <xsl:attribute name="type">rf_te_magnetic_field</xsl:attribute>
                                            <xsl:attribute name="rf_te_magnetic_field_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_te_magnetic_field_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_surface_current'">
                                            <xsl:attribute name="type">rf_te_surface_current</xsl:attribute>
                                            <xsl:attribute name="rf_te_surface_current_real">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_te_surface_current_imag">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_port'">
                                            <xsl:attribute name="type">rf_te_port</xsl:attribute>
                                            <xsl:attribute name="rf_te_port_power">
                                                <xsl:value-of select="@value_real"/>
                                            </xsl:attribute>
                                            <xsl:attribute name="rf_te_port_phase">
                                                <xsl:value-of select="@value_imag"/>
                                            </xsl:attribute>
                                        </xsl:if>
                                        <xsl:if test="@type='rf_matched_boundary'">
                                            <xsl:attribute name="type">rf_te_impedance</xsl:attribute>
                                            <xsl:attribute name="rf_te_impedance">428.75</xsl:attribute>
                                        </xsl:if>

                                        <!-- elasticity -->
                                        <xsl:if test="@typex='elasticity_free'">
                                            <xsl:if test="@typey='elasticity_free'">
                                                <xsl:attribute name="type">elasticity_free_free</xsl:attribute>
                                                <xsl:attribute name="elasticity_force_x">
                                                    <xsl:value-of select="@forcex"/>
                                                </xsl:attribute>
                                                <xsl:attribute name="elasticity_force_y">
                                                    <xsl:value-of select="@forcey"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>
                                        <xsl:if test="@typex='elasticity_fixed'">
                                            <xsl:if test="@typey='elasticity_fixed'">
                                                <xsl:attribute name="type">elasticity_fixed_fixed</xsl:attribute>
                                                <xsl:attribute name="elasticity_displacement_x">
                                                    <xsl:value-of select="@forcex"/>
                                                </xsl:attribute>
                                                <xsl:attribute name="elasticity_displacement_y">
                                                    <xsl:value-of select="@forcey"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>
                                        <xsl:if test="@typex='elasticity_free'">
                                            <xsl:if test="@typey='elasticity_fixed'">
                                                <xsl:attribute name="type">elasticity_free_fixed</xsl:attribute>
                                                <xsl:attribute name="elasticity_force_x">
                                                    <xsl:value-of select="@forcex"/>
                                                </xsl:attribute>
                                                <xsl:attribute name="elasticity_displacement_y">
                                                    <xsl:value-of select="@forcey"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>
                                        <xsl:if test="@typex='elasticity_fixed'">
                                            <xsl:if test="@typey='elasticity_free'">
                                                <xsl:attribute name="type">elasticity_fixed_free</xsl:attribute>
                                                <xsl:attribute name="elasticity_displacement_x">
                                                    <xsl:value-of select="@forcex"/>
                                                </xsl:attribute>
                                                <xsl:attribute name="elasticity_force_y">
                                                    <xsl:value-of select="@forcey"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <xsl:attribute name="id">
                                            <xsl:value-of select="@id"/>
                                        </xsl:attribute>
                                        <xsl:attribute name="name">
                                            <xsl:value-of select="@name"/>
                                        </xsl:attribute>

                                        <xsl:call-template name="edges">
                                            <xsl:with-param name="id" select="@id" />
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:for-each>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>

                    <!-- materials -->
                    <xsl:element name="materials">
                        <xsl:choose>
                            <xsl:when test="$current_version">
                                <!-- transformation from version 2.0 -->
                                <xsl:for-each select="/document/problems/problem/labels/label">
                                    <xsl:element name="material">
                                        <xsl:for-each select="@*">
                                            <xsl:attribute name="{name()}">
                                                <xsl:value-of select="."/>
                                            </xsl:attribute>
                                        </xsl:for-each>
                                        <xsl:call-template name="labels">
                                            <xsl:with-param name="id" select="@id" />
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:for-each>
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- transformation from version 1.0 -->
                                <xsl:variable name="physical_field" select="/document/problems/problem/@type" />
                                <xsl:for-each select="/document/problems/problem/labels/label">
                                    <xsl:element name="material">
                                        <!-- magnetic field -->
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

                                        <!-- electrostatic field -->
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

                                        <!-- temperature field -->
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

                                        <!-- acoustic field -->
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

                                        <!-- current field -->
                                        <xsl:if test="$physical_field='current'">
                                            <xsl:if test="@conductivity">
                                                <xsl:attribute name="current_conductivity">
                                                    <xsl:value-of select="@conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- rf field -->
                                        <xsl:if test="$physical_field='rf'">
                                            <xsl:if test="@permittivity">
                                                <xsl:attribute name="rf_te_permittivity">
                                                    <xsl:value-of select="@permittivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@permeability">
                                                <xsl:attribute name="rf_te_permeability">
                                                    <xsl:value-of select="@permittivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@conductivity">
                                                <xsl:attribute name="rf_te_conductivity">
                                                    <xsl:value-of select="@conductivity"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@current_density_real">
                                                <xsl:attribute name="rf_te_current_density_external_real">
                                                    <xsl:value-of select="@current_density_real"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@current_density_imag">
                                                <xsl:attribute name="rf_te_current_density_external_imag">
                                                    <xsl:value-of select="@current_density_imag"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <!-- elasticity -->
                                        <xsl:if test="$physical_field='elasticity'">
                                            <xsl:if test="@poisson_ratio">
                                                <xsl:attribute name="elasticity_poisson_ratio">
                                                    <xsl:value-of select="@poisson_ratio"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@temp">
                                                <xsl:variable name="temp_abs" select="@temp" />
                                                <xsl:variable name="temp_ref" select="@temp_ref" />
                                                <xsl:attribute name="elasticity_temperature_difference">
                                                    <xsl:value-of select="number($temp_abs - $temp_ref)"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@temp_ref">
                                                <xsl:attribute name="elasticity_temperature_reference">
                                                    <xsl:value-of select="@temp_ref"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@young_modulus">
                                                <xsl:attribute name="elasticity_young_modulus">
                                                    <xsl:value-of select="@young_modulus"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@alpha">
                                                <xsl:attribute name="elasticity_alpha">
                                                    <xsl:value-of select="@alpha"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@forcex">
                                                <xsl:attribute name="elasticity_volume_force_x">
                                                    <xsl:value-of select="@forcex"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                            <xsl:if test="@forcey">
                                                <xsl:attribute name="elasticity_volume_force_y">
                                                    <xsl:value-of select="@forcey"/>
                                                </xsl:attribute>
                                            </xsl:if>
                                        </xsl:if>

                                        <xsl:attribute name="id">
                                            <xsl:value-of select="@id"/>
                                        </xsl:attribute>
                                        <xsl:attribute name="name">
                                            <xsl:value-of select="@name"/>
                                        </xsl:attribute>

                                        <xsl:call-template name="labels">
                                            <xsl:with-param name="id" select="@id" />
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:for-each>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>
                </xsl:element>
            </xsl:element>
        </xsl:element>

        <!-- config -->
        <xsl:element name="config">
          <xsl:attribute name="Problem_StartupScript">
            <xsl:value-of select="/document/problems/problem/scriptstartup" />
          </xsl:attribute>
        </xsl:element>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
