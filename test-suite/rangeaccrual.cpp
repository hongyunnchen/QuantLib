/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "rangeaccrual.hpp"
#include "utilities.hpp"
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>

#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace range_accrual_test {

    struct CommonVars {
        // General settings
        Date referenceDate, today, settlement;
        Calendar calendar;

        // Volatility Stuctures
        std::vector<Handle<SwaptionVolatilityStructure> > swaptionVolatilityStructures;
        Handle<SwaptionVolatilityStructure> atmVol;
        Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCube1;
        Handle<SwaptionVolatilityStructure> flatSwaptionVolatilityCube2;
        Handle<SwaptionVolatilityStructure> swaptionVolatilityCubeBySabr;

        std::vector<Period> atmOptionTenors, optionTenors;
        std::vector<Period> atmSwapTenors, swapTenors;
        std::vector<Spread> strikeSpreads;

        Matrix atmVolMatrix, volSpreadsMatrix;
        std::vector<std::vector<Handle<Quote> > > volSpreads;

        DayCounter dayCounter;
        BusinessDayConvention optionBDC;
        Natural swapSettlementDays;
        bool vegaWeightedSmileFit;

        // Range Accrual valuation
        Rate infiniteLowerStrike, infiniteUpperStrike;
        Real gearing, correlation;
        Spread spread;
        Date startDate;
        Date endDate;
        Date paymentDate;
        Natural fixingDays;
        DayCounter rangeCouponDayCount;
        ext::shared_ptr<Schedule> observationSchedule;
        // Observation Schedule conventions
        Frequency observationsFrequency;
        BusinessDayConvention observationsConvention;

        // Term Structure
        RelinkableHandle<YieldTermStructure> termStructure;

        // indices and index conventions
        Frequency fixedLegFrequency;
        BusinessDayConvention fixedLegConvention;
        DayCounter fixedLegDayCounter;
        ext::shared_ptr<IborIndex> iborIndex;
        ext::shared_ptr<SwapIndex> swapIndexBase;
        ext::shared_ptr<SwapIndex> shortSwapIndexBase;

        // Range accrual pricers properties
        std::vector<bool> byCallSpread;
        Real flatVol;
        std::vector<ext::shared_ptr<SmileSection> > smilesOnExpiry;
        std::vector<ext::shared_ptr<SmileSection> > smilesOnPayment;

        //test parameters
        Real rateTolerance;
        Real priceTolerance;


        // cleanup
        SavedSettings backup;

        void createYieldCurve() {

            // Yield Curve
            std::vector<Date> dates;
            dates.emplace_back(39147);
            dates.emplace_back(39148);
            dates.emplace_back(39151);
            dates.emplace_back(39153);
            dates.emplace_back(39159);
            dates.emplace_back(39166);
            dates.emplace_back(39183);
            dates.emplace_back(39294);
            dates.emplace_back(39384);
            dates.emplace_back(39474);
            dates.emplace_back(39567);
            dates.emplace_back(39658);
            dates.emplace_back(39748);
            dates.emplace_back(39839);
            dates.emplace_back(39931);
            dates.emplace_back(40250);
            dates.emplace_back(40614);
            dates.emplace_back(40978);
            dates.emplace_back(41344);
            dates.emplace_back(41709);
            dates.emplace_back(42074);
            dates.emplace_back(42441);
            dates.emplace_back(42805);
            dates.emplace_back(43170);
            dates.emplace_back(43535);
            dates.emplace_back(43900);
            dates.emplace_back(44268);
            dates.emplace_back(44632);
            dates.emplace_back(44996);
            dates.emplace_back(45361);
            dates.emplace_back(45727);
            dates.emplace_back(46092);
            dates.emplace_back(46459);
            dates.emplace_back(46823);
            dates.emplace_back(47188);
            dates.emplace_back(47553);
            dates.emplace_back(47918);
            dates.emplace_back(48283);
            dates.emplace_back(48650);
            dates.emplace_back(49014);
            dates.emplace_back(49379);
            dates.emplace_back(49744);
            dates.emplace_back(50110);
            dates.emplace_back(53762);
            dates.emplace_back(57415);
            dates.emplace_back(61068);

            std::vector<Rate> zeroRates;
            zeroRates.push_back(0.02676568527);    zeroRates.push_back(0.02676568527);
            zeroRates.push_back(0.02676333038);    zeroRates.push_back(0.02682286201);
            zeroRates.push_back(0.02682038347);    zeroRates.push_back(0.02683030208);
            zeroRates.push_back(0.02700136766);    zeroRates.push_back(0.02932526033);
            zeroRates.push_back(0.03085568949);    zeroRates.push_back(0.03216370631);
            zeroRates.push_back(0.03321234116);    zeroRates.push_back(0.03404978072);
            zeroRates.push_back(0.03471117149);    zeroRates.push_back(0.03527141916);
            zeroRates.push_back(0.03574660393);    zeroRates.push_back(0.03691715582);
            zeroRates.push_back(0.03796468718);    zeroRates.push_back(0.03876457629);
            zeroRates.push_back(0.03942029708);    zeroRates.push_back(0.03999925325);
            zeroRates.push_back(0.04056663618);    zeroRates.push_back(0.04108743922);
            zeroRates.push_back(0.04156156761);    zeroRates.push_back(0.0419979179);
            zeroRates.push_back(0.04239486483);    zeroRates.push_back(0.04273799032);
            zeroRates.push_back(0.04305531203);    zeroRates.push_back(0.04336417578);
            zeroRates.push_back(0.04364017665);    zeroRates.push_back(0.04388153459);
            zeroRates.push_back(0.04408005012);    zeroRates.push_back(0.04424764425);
            zeroRates.push_back(0.04437504759);    zeroRates.push_back(0.04447696334);
            zeroRates.push_back(0.04456212318);    zeroRates.push_back(0.04464090072);
            zeroRates.push_back(0.0447068707);     zeroRates.push_back(0.04475921774);
            zeroRates.push_back(0.04477418345);    zeroRates.push_back(0.04477880755);
            zeroRates.push_back(0.04476692489);    zeroRates.push_back(0.04473779454);
            zeroRates.push_back(0.04468646066);    zeroRates.push_back(0.04430951558);
            zeroRates.push_back(0.04363922313);    zeroRates.push_back(0.04363601992);

            termStructure.linkTo( ext::shared_ptr<YieldTermStructure>(
                new ZeroCurve(dates, zeroRates, Actual365Fixed())));
        }

        void createVolatilityStructures() {

            // ATM swaptionvol matrix
            optionBDC = Following;

            atmOptionTenors = std::vector<Period>();
            atmOptionTenors.emplace_back(1, Months);
            atmOptionTenors.emplace_back(6, Months);
            atmOptionTenors.emplace_back(1, Years);
            atmOptionTenors.emplace_back(5, Years);
            atmOptionTenors.emplace_back(10, Years);
            atmOptionTenors.emplace_back(30, Years);

            atmSwapTenors = std::vector<Period>();
            atmSwapTenors.emplace_back(1, Years);
            atmSwapTenors.emplace_back(5, Years);
            atmSwapTenors.emplace_back(10, Years);
            atmSwapTenors.emplace_back(30, Years);

            atmVolMatrix = Matrix(atmOptionTenors.size(), atmSwapTenors.size());
            //atmVolMatrix[0][0]=0.1300; atmVolMatrix[0][1]=0.1560; atmVolMatrix[0][2]=0.1390; atmVolMatrix[0][3]=0.1220;
            //atmVolMatrix[1][0]=0.1440; atmVolMatrix[1][1]=0.1580; atmVolMatrix[1][2]=0.1460; atmVolMatrix[1][3]=0.1260;
            //atmVolMatrix[2][0]=0.1600; atmVolMatrix[2][1]=0.1590; atmVolMatrix[2][2]=0.1470; atmVolMatrix[2][3]=0.1290;
            //atmVolMatrix[3][0]=0.1640; atmVolMatrix[3][1]=0.1470; atmVolMatrix[3][2]=0.1370; atmVolMatrix[3][3]=0.1220;
            //atmVolMatrix[4][0]=0.1400; atmVolMatrix[4][1]=0.1300; atmVolMatrix[4][2]=0.1250; atmVolMatrix[4][3]=0.1100;
            //atmVolMatrix[5][0]=0.1130; atmVolMatrix[5][1]=0.1090; atmVolMatrix[5][2]=0.1070; atmVolMatrix[5][3]=0.0930;

            atmVolMatrix[0][0]=flatVol; atmVolMatrix[0][1]=flatVol; atmVolMatrix[0][2]=flatVol; atmVolMatrix[0][3]=flatVol;
            atmVolMatrix[1][0]=flatVol; atmVolMatrix[1][1]=flatVol; atmVolMatrix[1][2]=flatVol; atmVolMatrix[1][3]=flatVol;
            atmVolMatrix[2][0]=flatVol; atmVolMatrix[2][1]=flatVol; atmVolMatrix[2][2]=flatVol; atmVolMatrix[2][3]=flatVol;
            atmVolMatrix[3][0]=flatVol; atmVolMatrix[3][1]=flatVol; atmVolMatrix[3][2]=flatVol; atmVolMatrix[3][3]=flatVol;
            atmVolMatrix[4][0]=flatVol; atmVolMatrix[4][1]=flatVol; atmVolMatrix[4][2]=flatVol; atmVolMatrix[4][3]=flatVol;
            atmVolMatrix[5][0]=flatVol; atmVolMatrix[5][1]=flatVol; atmVolMatrix[5][2]=flatVol; atmVolMatrix[5][3]=flatVol;

            Size nRowsAtmVols = atmVolMatrix.rows();
            Size nColsAtmVols = atmVolMatrix.columns();


            //swaptionvolcube
            optionTenors = std::vector<Period>();
            optionTenors.emplace_back(1, Years);
            optionTenors.emplace_back(10, Years);
            optionTenors.emplace_back(30, Years);

            swapTenors = std::vector<Period>();
            swapTenors.emplace_back(2, Years);
            swapTenors.emplace_back(10, Years);
            swapTenors.emplace_back(30, Years);

            strikeSpreads = std::vector<Rate>();
            strikeSpreads.push_back(-0.020);
            strikeSpreads.push_back(-0.005);
            strikeSpreads.push_back(+0.000);
            strikeSpreads.push_back(+0.005);
            strikeSpreads.push_back(+0.020);

            Size nRows = optionTenors.size()*swapTenors.size();
            Size nCols = strikeSpreads.size();
            volSpreadsMatrix = Matrix(nRows, nCols);
            volSpreadsMatrix[0][0]=0.0599; volSpreadsMatrix[0][1]=0.0049;
            volSpreadsMatrix[0][2]=0.0000;
            volSpreadsMatrix[0][3]=-0.0001; volSpreadsMatrix[0][4]=0.0127;

            volSpreadsMatrix[1][0]=0.0729; volSpreadsMatrix[1][1]=0.0086;
            volSpreadsMatrix[1][2]=0.0000;
            volSpreadsMatrix[1][3]=-0.0024; volSpreadsMatrix[1][4]=0.0098;

            volSpreadsMatrix[2][0]=0.0738; volSpreadsMatrix[2][1]=0.0102;
            volSpreadsMatrix[2][2]=0.0000;
            volSpreadsMatrix[2][3]=-0.0039; volSpreadsMatrix[2][4]=0.0065;

            volSpreadsMatrix[3][0]=0.0465; volSpreadsMatrix[3][1]=0.0063;
            volSpreadsMatrix[3][2]=0.0000;
            volSpreadsMatrix[3][3]=-0.0032; volSpreadsMatrix[3][4]=-0.0010;

            volSpreadsMatrix[4][0]=0.0558; volSpreadsMatrix[4][1]=0.0084;
            volSpreadsMatrix[4][2]=0.0000;
            volSpreadsMatrix[4][3]=-0.0050; volSpreadsMatrix[4][4]=-0.0057;

            volSpreadsMatrix[5][0]=0.0576; volSpreadsMatrix[5][1]=0.0083;
            volSpreadsMatrix[5][2]=0.0000;
            volSpreadsMatrix[5][3]=-0.0043; volSpreadsMatrix[5][4]=-0.0014;

            volSpreadsMatrix[6][0]=0.0437; volSpreadsMatrix[6][1]=0.0059;
            volSpreadsMatrix[6][2]=0.0000;
            volSpreadsMatrix[6][3]=-0.0030; volSpreadsMatrix[6][4]=-0.0006;

            volSpreadsMatrix[7][0]=0.0533; volSpreadsMatrix[7][1]=0.0078;
            volSpreadsMatrix[7][2]=0.0000;
            volSpreadsMatrix[7][3]=-0.0045; volSpreadsMatrix[7][4]=-0.0046;

            volSpreadsMatrix[8][0]=0.0545; volSpreadsMatrix[8][1]=0.0079;
            volSpreadsMatrix[8][2]=0.0000;
            volSpreadsMatrix[8][3]=-0.0042; volSpreadsMatrix[8][4]=-0.0020;


            swapSettlementDays = 2;
            fixedLegFrequency = Annual;
            fixedLegConvention = Unadjusted;
            fixedLegDayCounter = Thirty360();
            ext::shared_ptr<SwapIndex> swapIndexBase(new
                EuriborSwapIsdaFixA(2*Years, termStructure));

            ext::shared_ptr<SwapIndex> shortSwapIndexBase(new
                EuriborSwapIsdaFixA(1*Years, termStructure));

            vegaWeightedSmileFit = false;

            // ATM Volatility structure
            std::vector<std::vector<Handle<Quote> > > atmVolsHandle;
            atmVolsHandle =
                std::vector<std::vector<Handle<Quote> > >(nRowsAtmVols);
            Size i;
            for (i=0; i<nRowsAtmVols; i++){
                atmVolsHandle[i] = std::vector<Handle<Quote> >(nColsAtmVols);
                for (Size j=0; j<nColsAtmVols; j++) {
                    atmVolsHandle[i][j] =
                        Handle<Quote>(ext::shared_ptr<Quote>(new
                            SimpleQuote(atmVolMatrix[i][j])));
                }
            }

            dayCounter = Actual365Fixed();

            atmVol = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(calendar,
                                             optionBDC,
                                             atmOptionTenors,
                                             atmSwapTenors,
                                             atmVolsHandle,
                                             dayCounter)));

            // Volatility Cube without smile
            std::vector<std::vector<Handle<Quote> > > parametersGuess(
                                      optionTenors.size()*swapTenors.size());
            for (i=0; i<optionTenors.size()*swapTenors.size(); i++) {
                parametersGuess[i] = std::vector<Handle<Quote> >(4);
                parametersGuess[i][0] =
                    Handle<Quote>(ext::shared_ptr<Quote>(
                                                       new SimpleQuote(0.2)));
                parametersGuess[i][1] =
                    Handle<Quote>(ext::shared_ptr<Quote>(
                                                       new SimpleQuote(0.5)));
                parametersGuess[i][2] =
                    Handle<Quote>(ext::shared_ptr<Quote>(
                                                       new SimpleQuote(0.4)));
                parametersGuess[i][3] =
                    Handle<Quote>(ext::shared_ptr<Quote>(
                                                       new SimpleQuote(0.0)));
            }
            std::vector<bool> isParameterFixed(4, false);
            isParameterFixed[1]=true;

            std::vector<std::vector<Handle<Quote> > > nullVolSpreads(nRows);
            for (i=0; i<optionTenors.size()*swapTenors.size(); i++){
                nullVolSpreads[i] = std::vector<Handle<Quote> >(nCols);
                for (Size j=0; j<strikeSpreads.size(); j++) {
                    nullVolSpreads[i][j] =
                        Handle<Quote>(ext::shared_ptr<Quote>(
                                                        new SimpleQuote(0.)));
                }
            }

            ext::shared_ptr<SwaptionVolCube1>
            flatSwaptionVolatilityCube1ptr(new SwaptionVolCube1(
                atmVol,
                optionTenors,
                swapTenors,
                strikeSpreads,
                nullVolSpreads,
                swapIndexBase,
                shortSwapIndexBase,
                vegaWeightedSmileFit,
                parametersGuess,
                isParameterFixed,
                false));
            flatSwaptionVolatilityCube1 = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(
                                             flatSwaptionVolatilityCube1ptr));
            flatSwaptionVolatilityCube1->enableExtrapolation();

            ext::shared_ptr<SwaptionVolCube2>
            flatSwaptionVolatilityCube2ptr(new SwaptionVolCube2(atmVol,
                                                             optionTenors,
                                                             swapTenors,
                                                             strikeSpreads,
                                                             nullVolSpreads,
                                                             swapIndexBase,
                                                             shortSwapIndexBase,
                                                             vegaWeightedSmileFit));
            flatSwaptionVolatilityCube2 = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(flatSwaptionVolatilityCube2ptr));
            flatSwaptionVolatilityCube2->enableExtrapolation();


            // Volatility Cube with smile
            volSpreads = std::vector<std::vector<Handle<Quote> > >(nRows);
            for (i=0; i<optionTenors.size()*swapTenors.size(); i++){
                volSpreads[i] = std::vector<Handle<Quote> >(nCols);
                for (Size j=0; j<strikeSpreads.size(); j++) {
                    volSpreads[i][j] =
                        Handle<Quote>(ext::shared_ptr<Quote>(new
                            SimpleQuote(volSpreadsMatrix[i][j])));
                }
            }

            ext::shared_ptr<SwaptionVolCube1>
            swaptionVolatilityCubeBySabrPtr(new SwaptionVolCube1(
                atmVol,
                optionTenors,
                swapTenors,
                strikeSpreads,
                volSpreads,
                swapIndexBase,
                shortSwapIndexBase,
                vegaWeightedSmileFit,
                parametersGuess,
                isParameterFixed,
                false));
            swaptionVolatilityCubeBySabr = Handle<SwaptionVolatilityStructure>(
            ext::shared_ptr<SwaptionVolatilityStructure>(swaptionVolatilityCubeBySabrPtr));
            swaptionVolatilityCubeBySabr->enableExtrapolation();

            swaptionVolatilityStructures = std::vector<Handle<SwaptionVolatilityStructure> >();
            //swaptionVolatilityStructures.push_back(atmVol);
            //swaptionVolatilityStructures.push_back(flatSwaptionVolatilityCube1);
            swaptionVolatilityStructures.push_back(flatSwaptionVolatilityCube2);
            swaptionVolatilityStructures.push_back(swaptionVolatilityCubeBySabr);
        }


        void createSmileSections() {

            std::vector<Rate> strikes(1000);
            for (Size i=0; i < 1000; ++i)
                strikes[i] = 0.003+i/1000.0;

            Real expiryBag[] = {
                    2.45489828353233, 2.10748097295326, 1.87317517200074, 1.69808302023488, 1.55911989073644, 1.44436083444893, 1.34687413874126, 1.26228953588707, 1.18769456816136, 1.12104324191799, 1.06085561121201, 1.00603120341767,
                    0.9557256903997, 0.90928131840481, 0.86618579845204, 0.82601854761258, 0.78844752673212, 0.75320077993188, 0.720053785498, 0.68882313132617, 0.65935702808872, 0.6315321469569, 0.60524729504558, 0.58041392858028, 0.55696247745247, 0.53482969610895,
                    0.51396815038482, 0.49433040611518, 0.47586902913511, 0.45854923439037, 0.44232991227137, 0.4271636286132, 0.41300927380629, 0.39981941368572, 0.38754661408661, 0.37613711628872, 0.3655403238495, 0.35570564032638, 0.34657298244381,
                    0.33809175375924, 0.3302113578301, 0.32288119821383, 0.31605668679542, 0.30969165432103, 0.30374530740885, 0.29818001495472, 0.29296130813214, 0.28805788039229, 0.28344158746397, 0.27908807980922, 0.27496889692908, 0.27106759497973,
                    0.26736456783968, 0.26384242298178, 0.26048629770105, 0.25728259420353, 0.25421897960636, 0.25128438593772, 0.24846932636464, 0.2457646302819, 0.24316239199534, 0.24065533826636, 0.23823714453963, 0.2359018024876, 0.23364393623824,
                    0.2314584861473, 0.22934134125381, 0.22728807436907, 0.22529520698763, 0.22335957683184, 0.22147738916851, 0.21964643040327, 0.21786353825847, 0.21612649913974, 0.21443341568048, 0.21278144183081, 0.21116931267966, 0.20959481463266,
                    0.20805636655099, 0.20655270352358, 0.20508161195607, 0.20364277562069, 0.20223398092308, 0.20085427917995, 0.19950303793576, 0.19817867605168, 0.19688024484442, 0.19560679563067, 0.1943576959549, 0.19313168090606, 0.19192843425636,
                    0.19074669109476, 0.18958645142124, 0.18844613409698, 0.18732573912199, 0.18622431781295, 0.18514155394211, 0.18407649882616, 0.18302915246512, 0.18199951485896, 0.18098473995782, 0.17998672512827, 0.17900452168702, 0.17804192436726,
                    0.17708375423623, 0.17614582268223, 0.17522718102195, 0.17430948804497, 0.17341298232831, 0.17253671518868, 0.17170630107512, 0.17079651379229, 0.1699635698566, 0.16919292279081, 0.16828977629107, 0.16750584765911, 0.16681330885154,
                    0.16630513083155, 0.16607713061225, 0.16458611669548, 0.16424269334159, 0.16421328415935, 0.16451654658696, 0.16511864425345, 0.16595981011106, 0.16697679860657, 0.16811585101976, 0.16933206300786, 0.17060013634959, 0.17189192677377,
                    0.17320174218061, 0.17451282249852, 0.17582390281642, 0.1771324533122, 0.17843309811383, 0.17972646967684, 0.18101193554569, 0.18228696589827, 0.18355314187341, 0.18481014724332, 0.18605671709696, 0.18729538125645, 0.18852360989966,
                    0.18974519775978, 0.19095508519256, 0.18650291447481, 0.18765809450407, 0.18881706926652, 0.18995801904631, 0.19109074690418, 0.19221588529567, 0.19333533158737, 0.19444655595715, 0.19554702858289, 0.19664623629757, 0.19773658963479,
                    0.19881113158372, 0.19988757089924, 0.20095167733189, 0.20200756184262, 0.20306123275898, 0.20411269008099, 0.20514675487586, 0.20617828984861, 0.20720729499923, 0.20822459972251, 0.20923494743493, 0.21023517585884, 0.21123160954956,
                    0.21223120551794, 0.21321910105898, 0.21419213389501, 0.21516706409764, 0.21614610526123, 0.217099215748, 0.21805643719574, 0.21900512049379, 0.21995127396971, 0.22088541079052, 0.22183124803868, 0.22275716293758, 0.22367390723126,
                    0.22459918967462, 0.22550360108543, 0.22641275591273, 0.22731337259035, 0.22821651909009, 0.22910859761802, 0.22998834326308, 0.23088326784091, 0.23174846700873, 0.23261746090975, 0.23348708726629, 0.23434849170092, 0.2352057851746, 0.23606497601486,
                    0.2369077230113, 0.2377479401856, 0.23859068718204, 0.239419203929, 0.24025088295363, 0.2410756049674, 0.24190317303107, 0.24272251917282, 0.24353016488722, 0.24434287024589, 0.2451457725438, 0.24595120466385, 0.2467370306624,
                    0.24752696762191, 0.24831563967036, 0.24911158495742, 0.24988065088437, 0.25065635759441, 0.25143459412658, 0.25219954909257, 0.25296102555314, 0.25372787788573, 0.25448049996885, 0.25523533564634, 0.25598416299626, 0.25673583639609,
                    0.25747770673516, 0.25822052575753, 0.25895354171916, 0.25969130109728, 0.26041419777039, 0.26113835935457, 0.26185746129449, 0.26258193910644, 0.26329250289668, 0.26400654519234, 0.26470730592184, 0.26541786971208, 0.26612843350232, 0.26681844248777,
                    0.26750623787885, 0.2682139556192, 0.26890111855475, 0.26958195693499, 0.27025710321543, 0.27094331746769, 0.27162320716463, 0.27229519116741, 0.27298172164743, 0.2736334670732, 0.27429849406513, 0.2749752214844, 0.2756275993657, 0.27628725048561,
                    0.27693614986148, 0.27759516852585, 0.27824723017938, 0.27889233482206, 0.27953332850377, 0.28016388666921, 0.2808014018455, 0.28144460912158, 0.28206251817637, 0.28270319563032, 0.28332142091289, 0.28396209836684,
                    0.28458981048238, 0.28520013007079, 0.28582183385878, 0.28641823942549, 0.28705543837401, 0.28765089525742, 0.28825900125147, 0.28886647478999, 0.28945750448468, 0.29006529425096, 0.2906613835899, 0.29127012203949, 0.291862732873, 0.2924480704679,
                    0.29303182692397, 0.29363012985727, 0.29420534816365, 0.29478025024227, 0.2953690663426, 0.29594112237132, 0.29650653761696, 0.29709598617281, 0.29766393124058, 0.29822650043632, 0.29881373539782, 0.29935132260005, 0.29993128432292, 0.3004916399243,
                    0.30104408983154, 0.30161646208803, 0.30215752779568, 0.30271788339706, 0.30325578682706, 0.30378198982971, 0.30433064500375, 0.30488119754438, 0.30545356980087, 0.30597060219831, 0.3064955402899, 0.30705494720798, 0.30757893661627, 0.30810513961892,
                    0.30863956454349, 0.30916892982381, 0.30970841439263, 0.31022671170113, 0.31075702566474, 0.31126773350686, 0.31179520142058, 0.31229389260758, 0.31280713027183, 0.31333902537427, 0.31383898147234, 0.31435759500861, 0.31485723487892, 0.3153663615822,
                    0.31588529134624, 0.31638587989984, 0.31688868204781, 0.31736618597449, 0.3178974486214, 0.31837495254809, 0.31888091697371, 0.31936790773338, 0.31985489849305, 0.32035453836335,
                    0.32084785367834, 0.3213190330497, 0.32179969925405, 0.32230250140202, 0.32278316760636, 0.32327964519901, 0.32373501318207, 0.32421251710876, 0.3246931833131, 0.32517068723979, 0.32563870433349, 0.32613201964848, 0.32657157624324, 0.32704908016993,
                    0.32750761043065, 0.32799776346798, 0.32844364461806, 0.32893379765539, 0.32936386741717, 0.32984137134386, 0.33028409021628, 0.33075526958765, 0.33120431301539, 0.33164703188781, 0.3321403472028, 0.33257990379756, 0.33302262266999, 0.33347482837539,
                    0.33392387180314, 0.33434129245428, 0.334799822715, 0.33527100208637, 0.33570107184815, 0.33615960210887, 0.33658334731534, 0.33699444341116, 0.3374592982272, 0.33789253026665, 0.33833524913907, 0.33876848117851, 0.33916376588603, 0.3396349452574,
                    0.3400397167979, 0.34048559794799, 0.34089669404381, 0.34133625063857, 0.34175367128971, 0.34217741649618, 0.3426043239803, 0.34302174463144, 0.34340438022832, 0.34384393682308, 0.34427716886253, 0.34470091406899, 0.34512149699779, 0.34553891764893,
                    0.34591206641283, 0.34635478528526, 0.34674690771512, 0.34715167925562, 0.34756277535144, 0.3479865205579, 0.34840710348671, 0.34877392769529, 0.34920399745707, 0.34962458038587,
                    0.34998191776147, 0.35039617613495, 0.35079778539779, 0.35117409643935, 0.35159784164582, 0.35198680179802, 0.35235678828426, 0.35279318260136, 0.3531947918642, 0.35354896696214, 0.35395373850264, 0.35433321182186, 0.35472533425172, 0.35510480757094,
                    0.35550325455612, 0.35586375420938, 0.35627168802755, 0.35666064817975, 0.35703695922131, 0.35740062115223, 0.35779590585975, 0.35815008095769, 0.35856117705351, 0.35891535215145, 0.35931379913663, 0.35967429878989, 0.36002847388783, 0.36045538137195,
                    0.36080955646989, 0.3611415956242, 0.36156217855301, 0.36192267820627, 0.36227369102655, 0.36263735295746, 0.36299469033306, 0.36334254087568, 0.3637378255832, 0.3640951629588, 0.36445566261206, 0.36486675870788, 0.36520828469518, 0.36558459573674,
                    0.36590714805808, 0.366270809989, 0.3666123359763, 0.36699813385084, 0.36733333528281, 0.36769383493607, 0.36807330825529, 0.36844013246387, 0.36876584706287, 0.36910421077251, 0.36949000864705, 0.36982521007903, 0.37020468339825, 0.37051458660894,
                    0.37085611259624, 0.37119763858354, 0.37158659873574, 0.37187436600282, 0.37222854110076, 0.37257955392103, 0.37291791763067, 0.37333217600415, 0.37359780732761, 0.37397728064683,
                    0.37428718385752, 0.37461606073418, 0.37500185860872, 0.37531176181942, 0.37559952908649, 0.376004300627, 0.37629206789407, 0.37664624299201, 0.37695298392505, 0.37733245724427, 0.37766133412092, 0.37799969783056, 0.37827797826466, 0.37863531564026,
                    0.37894838112861, 0.37926460889463, 0.37957451210533, 0.37990022670432, 0.38027653774588, 0.38060541462254, 0.38089634416728, 0.38123470787691, 0.38159520753017, 0.38183554063235, 0.38213911928772, 0.38248696983034, 0.3828474694836, 0.38313207447302,
                    0.38347992501563, 0.38379615278165, 0.38410605599235, 0.38445706881263, 0.3847448360797, 0.38504841473508, 0.38537412933408, 0.38570933076605, 0.38603820764271, 0.38633862402043, 0.38662322900984, 0.3869521058865, 0.38724935998656, 0.38758456141853,
                    0.38785651729731, 0.38821385467291, 0.38851427105062, 0.38874827959747, 0.38907399419647, 0.38943133157207, 0.3896432041753, 0.39002583977218, 0.39035155437118, 0.39060769886165, 0.39090179068405, 0.39121801845007, 0.3915247593831, 0.39180936437252,
                    0.39209396936193, 0.39241968396093, 0.39272326261631, 0.3930015430504, 0.39333674448238, 0.39361186263881, 0.39394073951547, 0.394168423507, 0.39445935305174, 0.39480087903904,
                    0.39502540075291, 0.39534795307425, 0.3956135843977, 0.39595194810734, 0.39626817587336, 0.39650534669787, 0.39683106129687, 0.3971378022299, 0.39739078444272, 0.39767222715447, 0.3979315339226, 0.39826989763224, 0.39848493251313, 0.39882645850043,
                    0.39912687487815, 0.39939883075692, 0.39968027346868, 0.39991744429319, 0.40027478166879, 0.4005435752699, 0.40082818025932, 0.40110329841575, 0.40135628062857, 0.40166618383926, 0.40187489416483, 0.40223855609575, 0.40246624008729, 0.40274768279904,
                    0.40307023512038, 0.40332637961085, 0.40353508993642, 0.40379755898222, 0.40412643585887, 0.40442368995893, 0.40473359316963, 0.40497708854946, 0.40522058392929, 0.40553364941765, 0.40574235974322, 0.40608388573052, 0.40629575833375, 0.40664360887637,
                    0.40681437187002, 0.40711478824773, 0.40743417829141, 0.40762391495102, 0.40793698043938, 0.40826585731603, 0.40849670358523, 0.40877498401932, 0.40902480395447, 0.40926197477899, 0.40951811926946, 0.40983118475782, 0.41010946519191, 0.41034979829409,
                    0.41058380684094, 0.41093165738356, 0.41109925809955, 0.41137437625598, 0.411690604022, 0.41194358623481, 0.41221554211358, 0.41234835777531, 0.41267723465197, 0.41290808092116,
                    0.41318003679994, 0.41338242257019, 0.41370181261386, 0.41395479482668, 0.41422991298311, 0.41450819341721, 0.41474852651938, 0.41502048239815, 0.41527662688863, 0.41558336782166, 0.41573199487169, 0.41606087174835, 0.41633598990478, 0.41653205111971,
                    0.41677238422189, 0.41702852871236, 0.41728151092517, 0.41745543619648, 0.41772106751994, 0.41806891806256, 0.41820489600194, 0.4185337728786, 0.41877094370311, 0.41904289958189, 0.41922947396384, 0.41952989034155, 0.41974492522245, 0.4200263679342,
                    0.42028251242467, 0.42042797719704, 0.42065882346623, 0.42101932311949, 0.4211932483908, 0.42139563416105, 0.42170237509409, 0.42188262492072, 0.42217671674311, 0.42240123845699, 0.42265105839214, 0.43466455122312, 0.43492069571359, 0.42340368047526,
                    0.43543298469454, 0.42384639934768, 0.43591048862123, 0.43616347083404, 0.43641329076919, 0.43664413703839, 0.4368971192512, 0.43715958829699, 0.43735881178958, 0.43760230716942, 0.43785212710457, 0.43807664881844, 0.43833279330892, 0.43856996413343,
                    0.4388102972356, 0.43905695489309, 0.43927831432931, 0.43953445881978, 0.43976530508897, 0.44001512502412, 0.44026494495928, 0.44045468161889, 0.44072347522, 0.44095748376685,
                    0.44121046597967, 0.44140336491694, 0.44168164535103, 0.4418587329, 0.44209590372452, 0.44237418415861, 0.44259238131716, 0.44281374075337, 0.44307620979917, 0.44327543329176, 0.44353157778223, 0.44370550305354, 0.44396480982167, 0.44418300698023,
                    0.4444391514707, 0.44467632229521, 0.44491665539739, 0.44510322977934, 0.44532458921555, 0.44560286964964, 0.44575782125499, 0.44602977713376, 0.44628275934658, 0.44647882056151, 0.44665590811048, 0.44694051309989, 0.44716187253611, 0.4474243415819,
                    0.44760459140853, 0.44780065262346, 0.44808525761287, 0.44830029249377, 0.44850584054168, 0.44870822631193, 0.44893591030346, 0.44914145835137, 0.44936598006524, 0.44956520355783, 0.44987194449087, 0.45006168115048, 0.45022928186647, 0.4504727772463,
                    0.45073208401444, 0.45094079434001, 0.45116215377622, 0.45137402637945, 0.45157008759438, 0.45181358297421, 0.4520286178551, 0.45223416590302, 0.45243338939561, 0.45263577516586, 0.45286978371271, 0.45305635809466, 0.45327455525321, 0.45357813390859,
                    0.45376154601288, 0.45397025633845, 0.4541568307204, 0.45434656738001, 0.45459006275984, 0.45479561080775, 0.45503278163226, 0.45525730334614, 0.45546917594937, 0.45568421083026,
                    0.45590873254413, 0.4560541973165, 0.45628188130803, 0.45652537668786, 0.45668349057087, 0.45693647278369, 0.45712937172096, 0.45732227065823, 0.45752149415082, 0.45773652903171, 0.45797053757856, 0.45814130057221, 0.45840693189566, 0.45858718172229,
                    0.45884648849043, 0.45904571198302, 0.45921647497667, 0.4594125361916, 0.45965603157143, 0.45982995684274, 0.46005447855661, 0.46024421521622, 0.46039600454391, 0.46067112270034, 0.46085769708229, 0.46102846007594, 0.46121503445789, 0.46145852983773,
                    0.46165775333032, 0.46186962593355, 0.46212260814636, 0.46226491064107, 0.46247045868898, 0.46267600673689, 0.46291001528374, 0.46308710283271, 0.46324521671572, 0.46348238754023, 0.46369109786581, 0.46390929502436, 0.46407373346269, 0.46428560606592,
                    0.46447534272553, 0.4646682416628, 0.46484532921177, 0.46503190359372, 0.46525642530759, 0.46542718830124, 0.46564538545979, 0.46580666162046, 0.46602169650135, 0.4662557050482, 0.46648971359505, 0.466676287977, 0.46684388869299, 0.46699251574302,
                    0.46721387517923, 0.46747001966971, 0.46765659405166, 0.467811545657, 0.46793803676341, 0.46817204531026, 0.46837443108051, 0.46859579051673, 0.46880133856464, 0.46897210155829,
                    0.46913970227428, 0.46930730299026, 0.4695507983701, 0.46970891225311, 0.46995240763294, 0.47015163112553, 0.47026231084364, 0.47049948166815, 0.4706702446618, 0.47087895498737, 0.4710275820374, 0.47122048097467, 0.4714323535779, 0.47161576568219,
                    0.4718371251184, 0.47202686177801, 0.47218497566102, 0.47234625182169, 0.47257393581322, 0.47271307603027, 0.4729407600218, 0.47316844401333, 0.47331074650804, 0.47352894366659, 0.47365859705066, 0.47385782054325, 0.47401277214859, 0.47429105258269,
                    0.47443651735506, 0.47466103906893, 0.47476539423171, 0.47502786327751, 0.47519862627116, 0.4753219550999, 0.4755528013691, 0.47568561703082, 0.47590065191172, 0.47607457718302, 0.47620106828943, 0.47645721277991, 0.47659319071929, 0.47686198432041,
                    0.47697582631617, 0.4772035103077, 0.47732367685879, 0.47753871173968, 0.47772212384397, 0.47794664555784, 0.47809843488553, 0.4782913338228, 0.47837039076431, 0.47855064059094, 0.47879729824843, 0.47898703490804, 0.47911985056977, 0.47933804772832,
                    0.47956889399751, 0.47974914382414, 0.47985033670927, 0.48004323564654, 0.48022348547317, 0.48032467835829, 0.48060295879239, 0.48071680078815, 0.48098875666693, 0.48109627410737,
                    0.48123857660208, 0.48145677376063, 0.48157377803406, 0.48183308480219, 0.48197854957456, 0.48213666345757, 0.48234537378314, 0.48246237805656, 0.4827058734364, 0.48290825920665, 0.48307269764498, 0.48318021508542, 0.48346165779718, 0.48353755246102,
                    0.48369250406637, 0.48383164428342, 0.48403719233133, 0.48424274037924, 0.48436606920799, 0.48459691547718, 0.48470759519528, 0.48484989768999, 0.48509655534749, 0.48525466923049, 0.48549816461033, 0.48557405927417, 0.4856910635476, 0.48589977387317,
                    0.48617172975194, 0.48630770769133, 0.48651958029456, 0.48662709773501, 0.48676940022971, 0.48694332550102, 0.48710776393935, 0.48729750059896, 0.48742082942771, 0.48764218886392, 0.48780978957991, 0.48795209207462, 0.48810388140231, 0.4883347276715,
                    0.48847703016621, 0.48866044227049, 0.48884701665244, 0.48893872270459, 0.48904624014504, 0.48926443730359, 0.48940990207595, 0.48964707290047, 0.48976091489623, 0.48999492344309, 0.49018149782504, 0.49019098465802, 0.49046610281445, 0.49061156758682,
                    0.49078549285813, 0.49090882168687, 0.49103215051562, 0.49124402311885, 0.49137367650292, 0.49153495266359, 0.49173733843384, 0.49195237331473, 0.49205672847752, 0.49222749147116,
                    0.49237611852119, 0.49256901745846, 0.49268285945423, 0.49288208294682, 0.49298643810961, 0.49321412210114, 0.49335010004052, 0.49352402531183, 0.49371376197144, 0.49388768724275, 0.49405212568108, 0.49414383173323, 0.49435570433646, 0.49453911644075,
                    0.49461501110459, 0.49478261182058, 0.49488696698337, 0.49517473425044, 0.49532336130047, 0.49545933923986, 0.4955194225154, 0.49572180828565, 0.49592103177824, 0.49606649655061, 0.4962182858783, 0.49638588659429, 0.49652502681133, 0.49667681613902,
                    0.49679382041245, 0.49704996490292, 0.49718910511997, 0.49734721900298, 0.49747687238704, 0.4976792581573, 0.49777096420944, 0.49799864820097, 0.49804292008821, 0.49820419624888, 0.49842871796276, 0.49851726173724, 0.49867221334259, 0.49881767811496,
                    0.49907698488309, 0.49920031371184, 0.49941534859273, 0.49951337920019, 0.49966516852788, 0.49976952369067, 0.49991182618537, 0.50009840056732, 0.50028181267161, 0.5003703564461, 0.50050000983017, 0.50070239560042, 0.50084469809512, 0.50109135575262,
                    0.50112930308454, 0.50133801341011, 0.50154039918036, 0.50164475434315, 0.50172064900699, 0.50195465755384, 0.50209696004855, 0.50221080204432, 0.50240686325925, 0.50259659991886,
                    0.50265984547206, 0.50281163479975, 0.50290334085189, 0.50308359067852, 0.50330495011473, 0.50338400705624, 0.50352947182861, 0.5037128839329, 0.50385834870526, 0.50402911169891, 0.50412714230638, 0.50430106757769};

            Real paymentBag[] = {
                    1.6617526454415, 1.4669124167142, 1.32415790098, 1.2120961731935, 1.1201668663866, 1.0424206605982, 0.9751732547411, 0.9160138132757, 0.8632670647314, 0.8157437931899, 0.7725528968054, 0.7330333400265, 0.6966731443381, 0.6630705038169, 0.6319111025389,
                    0.6029486723577, 0.5759823103116, 0.5508499978832, 0.5274286009992, 0.5056047066973, 0.4852940653485, 0.4664189080644, 0.4489047063269, 0.4326866527292, 0.4176999398641, 0.4038765199544, 0.3911451048524, 0.3794344064103, 0.3686698961103, 0.3587770454342,
                    0.3496780854936, 0.3413049685113, 0.3335864063394, 0.3264575915712, 0.3198563090958, 0.3137287687655, 0.3080244208027, 0.3026982240597, 0.2977103219812, 0.2930253945303, 0.2886123341517, 0.2844432736605, 0.2804945583529, 0.2767441537107, 0.273174237697,
                    0.2697679603859, 0.266511064148, 0.2633912355757, 0.2603990775953, 0.2575187123919, 0.2547472236322, 0.2520745661682, 0.2494942592591, 0.2469994981273, 0.244584774143, 0.2422449027139, 0.2399756713583, 0.2377725435579, 0.2356322789423, 0.2335506650299,
                    0.2315251095246, 0.2295526960931, 0.2276305084019, 0.225756278192, 0.2239274131669, 0.2221426171785, 0.2203989738936, 0.218695187164, 0.2170296368045, 0.21540070263, 0.2138067644552, 0.212246202095, 0.2107183674754, 0.2092216404111, 0.207755048791,
                    0.2063172964671, 0.2049074113281, 0.2035247453001, 0.2021680022349, 0.2008362100214, 0.1995290446225, 0.1982452098902, 0.1969843817873, 0.1957459122398, 0.1945285050996, 0.1933321603667, 0.19215590593, 0.1909994177523, 0.1898617237227, 0.1887428238411,
                    0.1876417459965, 0.1865581661517, 0.1854914362327, 0.1844415562396, 0.1834078780982, 0.1823907258456, 0.1813868591114, 0.180399194229, 0.1794264350501, 0.1784666373527, 0.1775214213218, 0.1765904629205, 0.1756776505931, 0.1747651623028, 0.1738730883457,
                    0.1730004566106, 0.172122316246, 0.1712668584738, 0.1704344073311, 0.1695851062626, 0.1687652925642, 0.1679765864212, 0.1672679174259, 0.1663641781355, 0.1656295861773, 0.165014239848, 0.1646185906283, 0.1645304525548, 0.1629251730839, 0.1627171413074,
                    0.1628402753807, 0.1632897147481, 0.1640194461508, 0.1649614218113, 0.1660589352486, 0.1672562520926, 0.1685167561584, 0.1698180888907, 0.1711395119191, 0.1724687118363, 0.173801476161, 0.1751297039671, 0.1764537192918, 0.1777673654313, 0.1790764750524,
                    0.1803729472291, 0.181660994443, 0.1829389965087, 0.1842088976485, 0.1854681055662, 0.186718888521, 0.1879580061426, 0.1891903189864, 0.1904125866821, 0.1916248092297, 0.1928302269996, 0.1940249515474, 0.1952125472804, 0.1963913940134, 0.1975601955984,
                    0.1987189520352, 0.1998725238795, 0.1951120957995, 0.196220302459, 0.1973171678222, 0.1984056082225, 0.1994934005487, 0.2005659631343, 0.2016369055347, 0.2026955345278, 0.2037538394838, 0.2047917301067, 0.2058393418406, 0.2068697796116, 0.2078934126049,
                    0.2089163975242, 0.2099244767398, 0.2109348242147, 0.2119335063563, 0.2129312163868, 0.21391661301, 0.2149003894481, 0.2158760649602, 0.2168452597316, 0.2178060295402, 0.2187635589784, 0.2197220605277, 0.22067148904, 0.2216082801079, 0.2225421548426,
                    0.2234692247995, 0.2243969428305, 0.2253159118615, 0.2262309924481, 0.2271337596274, 0.2280494882881, 0.2289418862823, 0.2298333121653, 0.2307247380484, 0.2315996380427, 0.2324810187777, 0.2333601312534, 0.2342246620626, 0.2350934053532, 0.2359469189032,
                    0.236808209342, 0.2376552421513, 0.238496766331, 0.2393373183995, 0.2401791666163, 0.2410038408701, 0.2418327276055, 0.2426548095631, 0.2434785117058, 0.2442895764042, 0.2450967526582, 0.2459032808381, 0.2467140214995, 0.2475079122351, 0.2483066635261,
                    0.2490927773729, 0.2498782431455, 0.2506491272516, 0.2514336209131, 0.2522058011673, 0.2529708526068, 0.2537372001944, 0.2545012795227, 0.2552695713325, 0.256016476698, 0.2567708349153, 0.257510611466, 0.2582555726093, 0.2590028020118, 0.2597399862663,
                    0.2604713378541, 0.2612017173308, 0.2619359852519, 0.2626640964694, 0.2633734135388, 0.2640940719045, 0.2648053331961, 0.2655204829322, 0.2662158664091, 0.2669216190712, 0.2676212150296, 0.2683191908028, 0.2690242953908, 0.2697144942752, 0.2703836307523,
                    0.2710796623033, 0.2717640285211, 0.2724451543685, 0.2731220677344, 0.2738015733966, 0.2744671454664, 0.275112951277, 0.2757924569392, 0.2764502521201, 0.2771064271158, 0.2777600098152, 0.2784090559962, 0.2790762482511, 0.2797204338765, 0.2803668877612,
                    0.2810110733866, 0.281636464864, 0.2822780581931, 0.2829177073001, 0.2835499035552, 0.2841730267733, 0.2848142960654, 0.285413116506, 0.2860365637612, 0.286665843683, 0.2872776256049, 0.2878845469713, 0.2885041057821, 0.289124960741, 0.2897202167741,
                    0.2903238977702, 0.2909259585811, 0.2915215386513, 0.2921174427585, 0.2927146430138, 0.2933073067507, 0.2938934897469, 0.2944903659652, 0.295054514443, 0.2956406974392, 0.2962297967686, 0.2968218124314, 0.2973921176129, 0.297958858387, 0.2985395327536,
                    0.2991065975647, 0.2996862998201, 0.3002384589277, 0.3008006631832, 0.3013589789944, 0.3019250716944, 0.3024749625427, 0.3030384629464, 0.3035870576465, 0.3041492619021, 0.3047001248615, 0.3052396465246, 0.3057801402989, 0.3063271148138, 0.3068698768473,
                    0.3074081023623, 0.3079554009143, 0.3084780726516, 0.3090282875369, 0.3095509592742, 0.3100729829375, 0.3106125046006, 0.3111494339675, 0.3116633567049, 0.3121727429238, 0.3126934704389, 0.3132252152132, 0.313724880321, 0.314244311688, 0.3147873977585,
                    0.3152822023109, 0.3157932087149, 0.3162896334524, 0.3168025840787, 0.3173041934088, 0.3177967297018, 0.3182963948097, 0.3187960599175, 0.3193219720252, 0.3198099717998, 0.3203089888336, 0.3208180510154, 0.3212995700493, 0.3217820611943, 0.322270709043,
                    0.3227697260768, 0.3232463845551, 0.3237428092926, 0.3241828515861, 0.3246818686198, 0.3251744049129, 0.3256474989839, 0.3261141123142, 0.3265904467555, 0.3270667811968, 0.3275366348975, 0.3280194500795, 0.3284860634098, 0.3289494363697, 0.3294095689593,
                    0.3298761822896, 0.3303460359902, 0.3307996878391, 0.3312792626508, 0.3317231933887, 0.3321638837561, 0.3326240163457, 0.3330938700464, 0.3335604833767, 0.3340011737441, 0.3344645467041, 0.3348890352198, 0.3353426870687, 0.3357930985473, 0.3362467503961,
                    0.3366679985415, 0.3371443329829, 0.3375558600172, 0.3379933100144, 0.3384404811226, 0.3388649696383, 0.3393315829686, 0.3397495907436, 0.3401805600001, 0.3406018081455, 0.3410587003647, 0.3414734677694, 0.3418979562852, 0.3423483677637, 0.3427728562794,
                    0.3431746622027, 0.3436088718295, 0.3440009566418, 0.3444286855279, 0.3448758566361, 0.3452582203373, 0.3457021510751, 0.3461396010723, 0.3465025225514, 0.3469561744003, 0.3473741821753, 0.3477565458765, 0.3481713132812, 0.3485763595749, 0.3489684443871,
                    0.3494156154953, 0.349820661789, 0.3502321888233, 0.350637235117, 0.3510293199292, 0.3514343662229, 0.3518037684427, 0.3522509395509, 0.3526203417707, 0.353031868805, 0.3534077517655, 0.3537998365778, 0.3542081232418, 0.3545872465726, 0.3549955332366,
                    0.3553746565675, 0.3557764624908, 0.3561296628589, 0.3565541513746, 0.356910592113, 0.3572799943328, 0.3576915213672, 0.3580803658091, 0.3584724506213, 0.3588386124708, 0.359211255061, 0.3596357435767, 0.3599921843151, 0.3603615865349, 0.3607569117175,
                    0.3611165928263, 0.3615021968978, 0.3618910413397, 0.3622604435596, 0.3625942017055, 0.3630219305916, 0.3633589291079, 0.3637412928091, 0.3640750509551, 0.3644736165081, 0.3648203361354, 0.3651864979849, 0.3655623809454, 0.3658896583507, 0.366291464274,
                    0.3666576261234, 0.3670011053804, 0.3673802287113, 0.3677107464869, 0.3680833890771, 0.3684527912969, 0.3687930301835, 0.3691624324034, 0.369492950179, 0.3698785542505, 0.3701961105448, 0.3705590320239, 0.3708960305402, 0.3712783942414, 0.3715927101653,
                    0.3719685931258, 0.3723250338642, 0.3726652727508, 0.3730022712672, 0.3733587120056, 0.3736859894108, 0.3740456705196, 0.3743826690359, 0.3747488308854, 0.3750955505127, 0.3754066260663, 0.3757274227308, 0.3760968249507, 0.3764208619856, 0.3767546201315,
                    0.3770980993886, 0.3774253767938, 0.3777558945694, 0.3780799316044, 0.3784493338242, 0.3787766112294, 0.3791136097458, 0.3794635697435, 0.3797616838156, 0.3800954419616, 0.3804713249221, 0.3807791601053, 0.3811226393623, 0.3814142726937, 0.3817739538025,
                    0.3820688275042, 0.3824058260206, 0.3827169015741, 0.383031217498, 0.3833163700887, 0.3836857723085, 0.3839644441585, 0.3843014426748, 0.3846611237836, 0.3849171130412, 0.3852800345203, 0.3855943504442, 0.3859313489605, 0.3862229822919, 0.3865599808082,
                    0.3868483737693, 0.3871464878414, 0.3874608037653, 0.3877653985781, 0.3881088778352, 0.3883745882038, 0.3887180674608, 0.3889805374591, 0.3893402185679, 0.3896059289365, 0.3899947733784, 0.3902475222656, 0.3905618381895, 0.3908826348541, 0.3911937104076,
                    0.3914918244797, 0.391828822996, 0.3920815718833, 0.3923991281775, 0.3927264055828, 0.3930342407659, 0.3932610666904, 0.3936175074288, 0.3939480252044, 0.3942234566841, 0.3944762055713, 0.3948132040877, 0.3950756740859, 0.3954353551947, 0.3956881040819,
                    0.3960121411169, 0.3962746111151, 0.3965727251873, 0.3968935218518, 0.3971624725908, 0.3974832692554, 0.3977749025868, 0.3980341322148, 0.3983290059165, 0.3986562833218, 0.3989057918387, 0.3991844636887, 0.3994631355388, 0.3997709707219, 0.4000237196092,
                    0.4003347951627, 0.4006037459017, 0.4009148214552, 0.4012258970087, 0.4014656844146, 0.4018124040419, 0.4020165473739, 0.4023470651496, 0.4026127755182, 0.4028752455165, 0.40318632107, 0.4034682332904, 0.4037145014369, 0.4039899329166, 0.4042977680998,
                    0.4045894014312, 0.404884275133, 0.4051046203167, 0.4053930132778, 0.4056684447575, 0.4059017514226, 0.4062031058651, 0.4065109410483, 0.4067410073431, 0.4070423617856, 0.4072951106728, 0.4075932247449, 0.4078621754839, 0.4080825206676, 0.4084162788136,
                    0.4086366239974, 0.4089185362177, 0.4092522943637, 0.409449956955, 0.4097480710271, 0.4100008199144, 0.4102956936162, 0.4105678847255, 0.4107558262057, 0.4111252284255, 0.4113941791645, 0.4116145243483, 0.4118672732355, 0.4121718680483, 0.4123792517507,
                    0.4126708850821, 0.4129463165618, 0.4132152673008, 0.413458295077, 0.4137078035939, 0.413931389148, 0.4141679361835, 0.414479011737, 0.414747962476, 0.4150007113632, 0.4152826235836, 0.4155386128412, 0.415778400247, 0.4160052261714, 0.416326022836,
                    0.4165139643163, 0.4167958765367, 0.4171199135716, 0.4172171246821, 0.417550882828, 0.4178619583815, 0.4181179476391, 0.4182864468973, 0.4185878013398, 0.4188178676346, 0.4190479339294, 0.419313644298, 0.4196247198515, 0.4198126613318, 0.4200362468859,
                    0.4202825150324, 0.4205968309563, 0.420807455029, 0.4210375213238, 0.4213356353959, 0.4215235768761, 0.4218573350221, 0.4220614783541, 0.4222591409454, 0.422589658721, 0.4227743598309, 0.4230368298292, 0.4232733768647, 0.4234937220485, 0.4237205479729,
                    0.4239311720456, 0.4242487283398, 0.4244787946346, 0.4247153416701, 0.4249907731498, 0.4251722338893, 0.4254282231469, 0.4256453279603, 0.4258948364772, 0.4261767486976, 0.4263614498075, 0.426607717954, 0.4267891786936, 0.4271650616541, 0.427340041653,
                    0.4276089923919, 0.4277807320205, 0.4280334809077, 0.4283089123874, 0.4285486997932, 0.428713958681, 0.4290120727532, 0.4291935334927, 0.4294430420096, 0.4296860697858, 0.4298578094143, 0.4301850868196, 0.430402191633, 0.4305739312615, 0.4307586323714,
                    0.4310761886656, 0.4312900531086, 0.4314844753296, 0.4317210223651, 0.4320223768076, 0.4321941164361, 0.4324177019902, 0.4326445279146, 0.432936161246, 0.4330301319862, 0.4333768516135, 0.433603677538, 0.4337721767961, 0.4340832523497, 0.434290636052,
                    0.4344008086439, 0.4346438364201, 0.4349095467887, 0.4350845267876, 0.435376160119, 0.4355673419696, 0.435719639376, 0.4359691478929, 0.4362705023354, 0.4485871500327, 0.4366334238145, 0.4368278460354, 0.4492967911392, 0.4495106555822, 0.4497504429881,
                    0.4499869900236, 0.4502267774294, 0.4504471226131, 0.4506383044637, 0.4508780918696, 0.4510919563126, 0.4512960996446, 0.4515196851987, 0.4517400303825, 0.4519798177883, 0.4522228455645, 0.4524367100075, 0.4526538148209, 0.4528482370419, 0.4530945051884,
                    0.4532921677797, 0.4535254744449, 0.4537296177769, 0.4539596840717, 0.4542124329589, 0.4543906533281, 0.4546174792526, 0.4548378244363, 0.4550192851759, 0.455252591841, 0.4554794177655, 0.4556576381347, 0.4558909447998, 0.4560983285021, 0.4563413562783,
                    0.45654225924, 0.456746402572, 0.456960267015, 0.4572000544209, 0.4573782747901, 0.4576213025663, 0.4578351670093, 0.4580393103413, 0.4582337325623, 0.4584670392274, 0.4586711825594, 0.4589077295949, 0.4590794692234, 0.4592998144072, 0.4594812751467,
                    0.4597016203305, 0.4599316866253, 0.4601163877352, 0.4603172906968, 0.4605343955102, 0.4607579810643, 0.4609815666184, 0.4611565466173, 0.4613704110603, 0.4615453910592, 0.4617916592057, 0.4619666392046, 0.4622129073511, 0.4623781662389, 0.4625985114227,
                    0.4627929336436, 0.4630132788274, 0.4632174221594, 0.4634118443803, 0.463632189564, 0.4638201310443, 0.4640145532653, 0.4642543406711, 0.4644325610403, 0.4646464254833, 0.4648505688153, 0.4649996258514, 0.4652815380718, 0.4654435565893, 0.4656509402916,
                    0.4658615643643, 0.4660365443632, 0.4662180051027, 0.4664837154713, 0.4666327725074, 0.4668077525063, 0.4670605013935, 0.4672290006517, 0.4674169421319, 0.4675919221308, 0.4678155076849, 0.4679969684244, 0.468188150275, 0.4683955339774, 0.4685769947169,
                    0.4688232628635, 0.4690176850844, 0.4692283091571, 0.4694454139705, 0.4695685480438, 0.4697888932275, 0.4700351613741, 0.4702004202619, 0.4704078039642, 0.4705147361858, 0.4707707254433, 0.4709748687753, 0.4711271661818, 0.4713377902545, 0.4715646161789,
                    0.4717331154371, 0.4719372587691, 0.4720506717313, 0.4722839783964, 0.4724427565435, 0.4727376302453, 0.4728802065407, 0.4730811095023, 0.4732463683901, 0.4734505117221, 0.473625491721, 0.4738004717199, 0.4739916535705, 0.4741795950507, 0.474377257642,
                    0.4745133531967, 0.4747304580101, 0.4749313609717, 0.4751484657851, 0.4753590898578, 0.4754854643014, 0.4757025691148, 0.4759067124468, 0.4760460483718, 0.4762696339259, 0.4763992487399, 0.4766260746644, 0.4768075354039, 0.4769533520696, 0.4771445339202,
                    0.4773551579929, 0.4775398591028, 0.477718079472, 0.4778768576191, 0.4780972028029, 0.4782819039128, 0.4784892876151, 0.478589739096, 0.478803603539, 0.4789915450193, 0.4791665250181, 0.4793998316833, 0.4795035235344, 0.4796882246444, 0.4799798579758,
                    0.4800738287159, 0.4802747316776, 0.480427029084, 0.4806246916753, 0.4808029120445, 0.4809649305619, 0.4811625931532, 0.4813732172259, 0.4815093127806, 0.4817199368533, 0.4818754746301, 0.4821055409249, 0.4822675594423, 0.4824328183301, 0.482607798329,
                    0.4827730572168, 0.4829772005488, 0.4831619016587, 0.4832785549913, 0.4834956598047, 0.4836479572111, 0.4838196968396, 0.4840011575791, 0.4842085412815, 0.4842895505402, 0.4845487801682, 0.4847691253519, 0.4849117016473, 0.4850866816461, 0.4852195368304,
                    0.4854463627549, 0.4855597757171, 0.4857606786788, 0.4859875046032, 0.4861041579358, 0.486282378305, 0.4864897620073, 0.4866128960806, 0.4867587127463, 0.4869563753376, 0.4871540379289, 0.4872901334836, 0.4874165079272, 0.4876109301482, 0.4877826697767,
                    0.4880094957011, 0.4881650334779, 0.4883140905139, 0.4884761090314, 0.4886316468082, 0.4888293093995, 0.4889621645838, 0.4891727886565, 0.4893186053222, 0.489493585321, 0.4896556038385, 0.4898791893926, 0.4900023234659, 0.4902323897607, 0.4903490430932,
                    0.4904754175369, 0.4906990030909, 0.490838339016, 0.4910262804962, 0.4911753375323, 0.4913924423457, 0.4915155764189, 0.4916581527143, 0.4918720171574, 0.4919757090085, 0.4921701312295, 0.4923613130801, 0.4925298123382, 0.4927015519668, 0.4928311667807,
                    0.4929834641871, 0.4931940882598, 0.4932459341854, 0.4935570097389, 0.493641259368, 0.4938454027, 0.4939782578843, 0.4941305552908, 0.494318496771, 0.494457832696, 0.4946652163984, 0.4947494660275, 0.4949860130629, 0.4951059067659, 0.49532949232,
                    0.4954461456525, 0.4955627989851, 0.4957637019468, 0.4959484030567, 0.4960844986113, 0.4962821612026, 0.4964376989794, 0.4965997174968, 0.4967908993475, 0.4968783893469, 0.497037167494, 0.497176503419, 0.4973385219365, 0.4975621074906, 0.4977014434156,
                    0.4978375389702, 0.4980189997098, 0.4982199026715, 0.4982685082267, 0.4984985745215, 0.4986281893355, 0.4988128904454, 0.499013793407, 0.4991725715541, 0.4992438597018, 0.4995030893297, 0.4995938196995, 0.4997266748838, 0.4999372989565, 0.5000312696967,
                    0.5002030093252, 0.5003455856205, 0.5004363159903, 0.5006890648775, 0.5007927567287, 0.5010066211718, 0.5010746689491, 0.5013436196881, 0.5014311096875, 0.501602849316, 0.5017259833893, 0.5018944826474, 0.5020370589428, 0.5022638848673, 0.502344894126,
                    0.5025069126434, 0.5025684796801, 0.502827709308, 0.5029800067144, 0.503096660047, 0.503375331897, 0.5034239374523, 0.5036216000436, 0.5038063011535, 0.5039294352267, 0.5040784922628, 0.5041757033733, 0.5043701255942, 0.5044478944826, 0.5047136048513,
                    0.5047136048513, 0.504924228924, 0.5050959685525, 0.5051899392926, 0.5053908422542, 0.505611187438, 0.5056954370671, 0.5058185711403, 0.505964387806, 0.5061393678049 };

            const std::vector<Rate> stdDevsOnExpiry(&expiryBag[0], &expiryBag[0]+LENGTH(expiryBag));
            const std::vector<Rate> stdDevsOnPayment(&paymentBag[0], &paymentBag[0]+LENGTH(paymentBag));

            //Create smiles on Expiry Date
            smilesOnExpiry = std::vector<ext::shared_ptr<SmileSection> >();
            smilesOnExpiry.push_back(ext::shared_ptr<SmileSection>(
                new FlatSmileSection(startDate, flatVol, rangeCouponDayCount)));
            Real dummyAtmLevel = 0;
            smilesOnExpiry.push_back(ext::shared_ptr<SmileSection>(new
                InterpolatedSmileSection<Linear>(startDate,
                                                 strikes,
                                                 stdDevsOnExpiry,
                                                 dummyAtmLevel,
                                                 rangeCouponDayCount)));
            //smilesOnExpiry.push_back(
            //    swaptionVolatilityStructures_[0]->smileSection(startDate,
            //                                                   Period(6, Months)));
            //Create smiles on Payment Date
            smilesOnPayment = std::vector<ext::shared_ptr<SmileSection> >();
            smilesOnPayment.push_back(ext::shared_ptr<SmileSection>(
                new FlatSmileSection(endDate, flatVol, rangeCouponDayCount)));
            smilesOnPayment.push_back(ext::shared_ptr<SmileSection>(new
                InterpolatedSmileSection<Linear>(endDate,
                                                 strikes,
                                                 stdDevsOnPayment,
                                                 dummyAtmLevel,
                                                 rangeCouponDayCount)));
            //vars.smilesOnPayment.push_back(
            //    swaptionVolatilityStructures_[0]->smileSection(vars.endDate,
            //                                                   Period(6, Months)));

            QL_REQUIRE(smilesOnExpiry.size()==smilesOnPayment.size(),
                       "smilesOnExpiry.size()!=smilesOnPayment.size()");
        }

        CommonVars() {

            //General Settings
            calendar = TARGET();
            today = Date(39147); // 6 Mar 2007
            Settings::instance().evaluationDate() = today;
            settlement = today;
            //create Yield Curve
            createYieldCurve();
            referenceDate = termStructure->referenceDate();
            // Ibor index
            iborIndex =
                ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));

            // create Volatility Structures
            flatVol = 0.1;
            createVolatilityStructures();

            // Range Accrual valuation
            gearing = 1.0;
            spread = 0.0;
            infiniteLowerStrike = 1.e-9;
            infiniteUpperStrike = 1.0;
            correlation = 1.0;

            startDate = Date(42800); //6 Mar 2017
            endDate = Date(42984);   //6 Sep 2017
            paymentDate = endDate;   //6 Sep 2017
            fixingDays = 2;
            rangeCouponDayCount = iborIndex->dayCounter();

            // observations schedule
            observationsConvention = ModifiedFollowing;
            observationsFrequency = Daily;//Monthly;
            observationSchedule = ext::make_shared<Schedule>(startDate,endDate,
                                         Period(observationsFrequency),
                                         calendar,observationsConvention,
                                         observationsConvention,
                                         DateGeneration::Forward, false);
            // Range accrual pricers properties
            byCallSpread = std::vector<bool>();
            byCallSpread.push_back(true);
            byCallSpread.push_back(false);

            std::vector<Rate> strikes;

            //Create smiles sections
            createSmileSections();

            //test parameters
            rateTolerance = 2.0e-8;
            priceTolerance = 2.0e-4;
        }
    };

}


//******************************************************************************************//
//******************************************************************************************//
void RangeAccrualTest::testInfiniteRange()  {

    BOOST_TEST_MESSAGE("Testing infinite range accrual floaters...");

    using namespace range_accrual_test;

    CommonVars vars;

    //Coupon
    RangeAccrualFloatersCoupon coupon(vars.paymentDate,
                                      1.0,
                                      vars.iborIndex,
                                      vars.startDate,
                                      vars.endDate,
                                      vars.fixingDays,
                                      vars.rangeCouponDayCount,
                                      vars.gearing, vars.spread,
                                      vars.startDate, vars.endDate,
                                      vars.observationSchedule,
                                      vars.infiniteLowerStrike,
                                      vars.infiniteUpperStrike);

    Date fixingDate = coupon.fixingDate();

    for (Size z = 0; z < vars.smilesOnPayment.size(); z++) {
        for (Size i = 0; i < vars.byCallSpread.size(); i++){
            ext::shared_ptr<RangeAccrualPricer> bgmPricer(new
                RangeAccrualPricerByBgm(vars.correlation,
                                        vars.smilesOnExpiry[z],
                                        vars.smilesOnPayment[z],
                                        true,
                                        vars.byCallSpread[i]));

                coupon.setPricer(bgmPricer);

                //Computation
                const Rate rate = coupon.rate();
                const Rate indexfixing = vars.iborIndex->fixing(fixingDate);
                const Rate difference =  rate-indexfixing;

                if (std::fabs(difference) > vars.rateTolerance) {
                    BOOST_ERROR("\n" <<
                                "i:\t" << i << "\n"
                                "fixingDate:\t" << fixingDate << "\n"
                                "startDate:\t" << vars.startDate << "\n"
                                "range accrual rate:\t" << io::rate(rate) << "\n"
                                "index fixing:\t" << io::rate(indexfixing) << "\n"
                                "difference:\t" << io::rate(difference) << "\n"
                                "tolerance: \t" << io::rate(vars.rateTolerance));
                }
        }
    }
}

void RangeAccrualTest::testPriceMonotonicityWithRespectToLowerStrike() {

    BOOST_TEST_MESSAGE(
            "Testing price monotonicity with respect to the lower strike...");

    using namespace range_accrual_test;

    CommonVars vars;

    for (Size z = 0; z < vars.smilesOnPayment.size(); z++) {
        for (Size i = 0; i < vars.byCallSpread.size(); i++){
            ext::shared_ptr<RangeAccrualPricer> bgmPricer(new
                RangeAccrualPricerByBgm(vars.correlation,
                                        vars.smilesOnExpiry[z],
                                        vars.smilesOnPayment[z],
                                        true,
                                        vars.byCallSpread[i]));

            Real effectiveLowerStrike;
            Real previousPrice = 100.;

            for (Size k = 1; k < 100; k++){
                effectiveLowerStrike = 0.005 + k*0.001;
                RangeAccrualFloatersCoupon coupon(
                                            vars.paymentDate,
                                            1.,
                                            vars.iborIndex,
                                            vars.startDate,
                                            vars.endDate,
                                            vars.fixingDays,
                                            vars.rangeCouponDayCount,
                                            vars.gearing, vars.spread,
                                            vars.startDate, vars.endDate,
                                            vars.observationSchedule,
                                            effectiveLowerStrike,
                                            vars.infiniteUpperStrike);

                coupon.setPricer(bgmPricer);

                //Computation
                const Rate price = coupon.price(vars.termStructure);

                if (previousPrice <= price) {
                    BOOST_ERROR("\n" <<
                                "i:\t" << i << "\n"
                                "k:\t" << k << "\n"
                                "Price at lower strike\t" << effectiveLowerStrike-0.001 <<
                                ": \t" << previousPrice << "\n"
                                "Price at lower strike\t" << effectiveLowerStrike <<
                                ": \t" << price << "\n");
                }
                previousPrice = price;
            }
        }
    }
}


void RangeAccrualTest::testPriceMonotonicityWithRespectToUpperStrike() {

    BOOST_TEST_MESSAGE(
            "Testing price monotonicity with respect to the upper strike...");

    using namespace range_accrual_test;

    CommonVars vars;

    for (Size z = 0; z < vars.smilesOnPayment.size(); z++) {
        for (Size i = 0; i < vars.byCallSpread.size(); i++){
            ext::shared_ptr<RangeAccrualPricer> bgmPricer(new
                RangeAccrualPricerByBgm(vars.correlation,
                                        vars.smilesOnExpiry[z],
                                        vars.smilesOnPayment[z],
                                        true,
                                        vars.byCallSpread[i]));

            Real effectiveUpperStrike;
            Real previousPrice = 0.;

            for (Size k = 1; k < 95; k++){
                effectiveUpperStrike = 0.006 + k*0.001;
                RangeAccrualFloatersCoupon coupon(
                                            vars.paymentDate,
                                            1.,
                                            vars.iborIndex,
                                            vars.startDate,
                                            vars.endDate,
                                            vars.fixingDays,
                                            vars.rangeCouponDayCount,
                                            vars.gearing, vars.spread,
                                            vars.startDate, vars.endDate,
                                            vars.observationSchedule,
                                            .004,
                                            effectiveUpperStrike);

                coupon.setPricer(bgmPricer);

                //Computation
                const Rate price = coupon.price(vars.termStructure);

                if (previousPrice > price) {
                    BOOST_ERROR("\n" <<
                                "i:\t" << i << "\n"
                                "k:\t" << k << "\n"
                                "Price at upper strike\t" << effectiveUpperStrike-0.001 <<
                                ": \t" << previousPrice << "\n"
                                "Price at upper strike\t" << effectiveUpperStrike <<
                                ": \t" << price << "\n");
                }
                previousPrice = price;
            }
        }
    }
}


test_suite* RangeAccrualTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Range Accrual tests");
    suite->add(QUANTLIB_TEST_CASE(&RangeAccrualTest::testInfiniteRange));
    suite->add(QUANTLIB_TEST_CASE(
           &RangeAccrualTest::testPriceMonotonicityWithRespectToLowerStrike));
    suite->add(QUANTLIB_TEST_CASE(
           &RangeAccrualTest::testPriceMonotonicityWithRespectToUpperStrike));
    return suite;
}

