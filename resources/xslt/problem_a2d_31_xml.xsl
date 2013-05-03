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
                          <xsl:value-of select ="solver/@linearity_type"/>`
                        </xsl:attribute>

                        <xsl:copy-of select="refinement_edges"/>
                        <xsl:copy-of select="refinement_labels"/>
                        <xsl:copy-of select="polynomial_orders"/>
                        <xsl:copy-of select="boundaries"/>
                        <xsl:copy-of select="materials"/>

                        <!-- field config -->
                        <xsl:element name="field_config">
                           <!-- space config -->
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">SpaceNumberOfRefinements</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="@number_of_refinements" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">SpacePolynomialOrder</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="@polynomial_order" />
                               </xsl:attribute>
                           </xsl:element>

                            <!-- transient config -->
                            <xsl:element name="field_item">
                                <xsl:attribute name="field_key">TransientTimeSkip</xsl:attribute>
                                <xsl:attribute name="field_value">
                                    <xsl:value-of select="@time_skip" />
                                </xsl:attribute>
                            </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">TransientInitialCondition</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="@initial_condition" />
                               </xsl:attribute>
                           </xsl:element>

                           <!-- nonlinearity config -->
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">NonlinearTolerance</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@nonlinear_tolerance" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">NonlinearSteps</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@nonlinear_steps" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">NewtonDampingType</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:variable name="automatic_damping" select="./solver/@newton_automatic_damping" />
                                   <xsl:variable name="fixed_damping" select="./solver/@newton_damping_coeff" />
                                   <xsl:choose>
                                       <xsl:when test="$automatic_damping=1">0</xsl:when>
                                       <xsl:when test="$automatic_damping=0 and $fixed_damping!=1">1</xsl:when>
                                       <xsl:otherwise>2</xsl:otherwise>
                                   </xsl:choose>
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">NewtonDampingCoeff</xsl:attribute>
                               <xsl:attribute name="field_value">
                               <xsl:variable name="automatic_damping" select="./solver/@newton_automatic_damping" />
                               <xsl:choose>
                                   <xsl:when test="$automatic_damping=1">
                                       <xsl:value-of select="./solver/@newton_automatic_damping_coeff" />
                                   </xsl:when>
                                   <xsl:otherwise>
                                       <xsl:value-of select="./solver/@newton_damping_coeff" />
                                   </xsl:otherwise>
                               </xsl:choose>
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">NewtonDampingNumberToIncrease</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@newton_damping_number_to_increase" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">PicardAndersonAcceleration</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@picard_anderson_acceleration" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">PicardAndersonBeta</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@picard_anderson_beta" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">PicardAndersonNumberOfLastVectors</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./solver/@picard_anderson_vectors" />
                               </xsl:attribute>
                           </xsl:element>

                           <!-- adaptivity config -->
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">AdaptivitySteps</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./adaptivity/@adaptivity_steps" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">AdaptivityTolerance</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./adaptivity/@adaptivity_tolerance" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">AdaptivityTransientBackSteps</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./adaptivity/@adaptivity_back_steps" />
                               </xsl:attribute>
                           </xsl:element>
                           <xsl:element name="field_item">
                               <xsl:attribute name="field_key">AdaptivityTransientRedoneEach</xsl:attribute>
                               <xsl:attribute name="field_value">
                                   <xsl:value-of select="./adaptivity/@adaptivity_redone_each" />
                               </xsl:attribute>
                           </xsl:element>
                        </xsl:element>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>

            <xsl:copy-of select="problem:document/problem/couplings"/>

            <!-- problem config -->
            <xsl:element name="problem_config">
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">Frequency</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@frequency" />
                   </xsl:attribute>
               </xsl:element>
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">TimeMethod</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@time_method" />
                   </xsl:attribute>
               </xsl:element>
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">TimeMethodTolerance</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@time_method_tolerance" />
                   </xsl:attribute>
               </xsl:element>
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">TimeOrder</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@time_order" />
                   </xsl:attribute>
               </xsl:element>
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">TimeSteps</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@time_steps" />
                   </xsl:attribute>
               </xsl:element>
               <xsl:element name="field_item">
                   <xsl:attribute name="field_key">TimeTotal</xsl:attribute>
                   <xsl:attribute name="field_value">
                       <xsl:value-of select="problem:document/problem/@time_total" />
                   </xsl:attribute>
               </xsl:element>
            </xsl:element>

            <!-- couplings -->
            <xsl:copy-of select="couplings"/>
        </xsl:element>

        <!-- config -->
        <xsl:copy-of select="problem:document/config"/>
    </xsl:element>
</xsl:template>
</xsl:stylesheet>
