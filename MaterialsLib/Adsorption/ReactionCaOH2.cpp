/**
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include <cassert>
#include <logog/include/logog.hpp>
#include "ReactionCaOH2.h"
#include "Adsorption.h"

namespace Adsorption
{

const double ReactionCaOH2::R = GAS_CONST;
const double ReactionCaOH2::reaction_enthalpy = -1.12e+05;
const double ReactionCaOH2::reaction_entropy  = -143.5;
const double ReactionCaOH2::M_carrier = M_N2;
const double ReactionCaOH2::M_react   = M_H2O;

const double ReactionCaOH2::tol_l   = 1e-4;
const double ReactionCaOH2::tol_u   = 1.0 - 1e-4;
const double ReactionCaOH2::tol_rho = 0.1;

const double ReactionCaOH2::rho_low = 1656.0;
const double ReactionCaOH2::rho_up = 2200.0;


double
ReactionCaOH2::get_enthalpy(const double, const double, const double) const
{
    return - reaction_enthalpy/M_react;
}

double
ReactionCaOH2::get_reaction_rate(const double, const double, const double, const double) const
{
    ERR("get_reaction_rate do not call directly");
    std::abort();
    // TODO: error
    return -1.0;
}


double ReactionCaOH2::getReactionRate(double const solid_density)
{
    rho_s = solid_density;
    calculate_qR();
    return qR;
}

void ReactionCaOH2::update_param(
        double T_solid,
        double p_gas,
        double x_react,
        double rho_s_initial)
{
    T_s     = T_solid;
    this->p_gas = p_gas / 1e5; // convert Pa to bar
    this->x_react = x_react;
    rho_s   = rho_s_initial;
}

void ReactionCaOH2::calculate_qR()
{
    //Convert mass fraction into mole fraction
    // const double mol_frac_react = get_mole_fraction(x_react);
    const double mol_frac_react = AdsorptionReaction::get_molar_fraction(x_react, M_react, M_carrier);

    p_r_g = std::max(mol_frac_react * p_gas, 1.0e-3); //avoid illdefined log
    set_chemical_equilibrium();
    const double dXdt = Ca_hydration();
    qR = (rho_up - rho_low) * dXdt;
}

//determine equilibrium temperature and pressure according to van't Hoff
void ReactionCaOH2::set_chemical_equilibrium()
{
    X_D = (rho_s - rho_up - tol_rho)/(rho_low - rho_up - 2.0*tol_rho) ;
    X_D = (X_D < 0.5) ? std::max(tol_l,X_D) : std::min(X_D,tol_u); //constrain to interval [tol_l;tol_u]

    X_H = 1.0 - X_D;

    //calculate equilibrium
    // using the p_eq to calculate the T_eq - Clausius-Clapeyron
    T_eq = (reaction_enthalpy/R) / ((reaction_entropy/R) + std::log(p_r_g)); // unit of p in bar
    //Alternative: Use T_s as T_eq and calculate p_eq - for Schaube kinetics
    p_eq = exp((reaction_enthalpy/R)/T_s - (reaction_entropy/R));
}


double ReactionCaOH2::Ca_hydration()
{
    double dXdt;
        // step 3, calculate dX/dt
#ifdef SIMPLE_KINETICS
    if ( T_s < T_eq ) // hydration - simple model
#else
    if ( p_r_g > p_eq ) // hydration - Schaube model
#endif
    {
        //X_H = max(tol_l,X_H); //lower tolerance to avoid oscillations at onset of hydration reaction. Set here so that no residual reaction rate occurs at end of hydration.
#ifdef SIMPLE_KINETICS // this is from P. Schmidt
        dXdt = -1.0*(1.0-X_H) * (T_s - T_eq) / T_eq * 0.2 * conversion_rate::x_react;
#else //this is from Schaube
        if (X_H == tol_u || rho_s == rho_up)
            dXdt = 0.0;
        else if ( (T_eq-T_s) >= 50.0)
            dXdt = 13945.0 * exp(-89486.0/R/T_s) * std::pow(p_r_g/p_eq - 1.0,0.83) * 3.0 * (X_D) * std::pow(-1.0*log(X_D),0.666);
        else
            dXdt = 1.0004e-34 * exp(5.3332e4/T_s) * std::pow(p_r_g, 6.0) * (X_D);
#endif
    }
    else // dehydration
    {
        //X_D = max(tol_l,X_D); //lower tolerance to avoid oscillations at onset of dehydration reaction. Set here so that no residual reaction rate occurs at end of dehydration.
#ifdef SIMPLE_KINETICS // this is from P. Schmidt
        dXdt = -1.0* (1.0-X_D) * (T_s - T_eq) / T_eq * 0.05;
#else
        if (X_D == tol_u || rho_s == rho_low)
            dXdt = 0.0;
        else if (X_D < 0.2)
            dXdt = -1.9425e12 * exp( -1.8788e5/R/T_s ) * std::pow(1.0-p_r_g/p_eq,3.0)*(X_H);
        else
            dXdt = -8.9588e9 * exp( -1.6262e5/R/T_s ) * std::pow(1.0-p_r_g/p_eq,3.0)*2.0 * std::pow(X_H, 0.5);
#endif
    }
    return dXdt;
}









}