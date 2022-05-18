/*
Module : AADynamicalTime.cpp
Purpose: Implementation for the algorithms which provides for conversion between Universal Time (both UT1 and UTC) and Terrestrial Time (TT) aka Terrestrial Dynamical Time (TDT) 
         aka Ephemeris Time (ET)
Created: PJN / 29-12-2003
History: PJN / 01-02-2005 1. Fixed a problem with the declaration of the variable "Index" in the function 
                          CAADynamicalTime::DeltaT. Thanks to Mika Heiskanen for reporting this problem.
         PJN / 26-01-2007 1. Update to fit in with new layout of CAADate class
         PJN / 28-01-2007 1. Further updates to fit in with new layout of CAADate class
         PJN / 08-05-2011 1. Fixed a compilation issue on GCC where size_t was undefined in various methods. Thanks to 
                          Carsten A. Arnholm and Andrew Hammond for reporting this bug.
         PJN / 01-05-2012 1. Updated CAADynamicalTime::DeltaT to use new polynomical expressions from Espenak & Meeus 2006.
                          References used: http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html and 
                          http://www.staff.science.uu.nl/~gent0113/deltat/deltat_old.htm (Espenak & Meeus 2006 section). 
                          Thanks to Murphy Chesney for prompting this update.
         PJN / 02-05-2012 1. To further improve the accuracy of the CAADynamicalTime::DeltaT method, the code now uses a 
                          lookup table between the dates of 1 February 1973 to 1 April 2012 (for observed values) and predicted 
                          values from April 2012 till April 2015. These values are as provided by IERS Rapid
                          Service/Prediction Center at http://maia.usno.navy.mil/ser7/deltat.data and 
                          http://maia.usno.navy.mil/ser7/deltat.preds. This lookup table will of course need to be kept up to 
                          date as IERS update this information. As currently coded there is a single discontinuity of c. one second 
                          in early April 2015. At this point http://maia.usno.navy.mil/ser7/deltat.preds indicates an error value 
                          for DeltaT of about 0.9 seconds anyway.
                          2. A new CAADynamicalTime::CumulativeLeapSeconds has been provided. This method takes as input the Julian
                          Day value and returns the cumulative total of Leap seconds which have been applied to this point. For more
                          information about leap seconds please see http://en.wikipedia.org/wiki/Leap_second. Using this method you 
                          can now implement code which converts from Terrestial Time to Coordinated Universal time as follows:
                          
                          double TerrestialTime = some calculation using AA+ algorithms(JD);
                          double DeltaT = CAADynamicalTime::DeltaT(JD);
                          double UniversalTime1 = TerrestialTime - DeltaT/86400.0; //The time of the event using the UT1 time scale
                          double TerrestialAtomicTime = TerrestialTime - (32.184/86400.0); //The time of the event using the TAI time scale
                          double CumulativeLeapSeconds = CAADynamicalTime::CumulativeLeapSeconds(JD);
                          double CoordinatedUniversalTime = (DeltaT - CumulativeLeapSeconds - 32.184)/86400.0 + UniversalTime1; //The time of the event using the UTC time scale
         PJN / 13-10-2012 1. Fixed a typo in the spelling of Coefficient throughout the AADynamicalTime.cpp module
         PJN / 04-08-2013 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st April 2013
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2023
         PJN / 28-10-2013 1. Addition of a TT2UTC method which converts from TT to UTC.
                          2. Addition of a UTC2TT method which converts from UTC to TT.
                          3. Addition of a TT2TAI method which converts from TT to TAI.
                          4. Addition of a TAI2TT method which converts from TAI to TT.
                          5. Addition of a TT2UT1 method which converts from TT to UT1.
                          6. Addition of a UT12TT method which converts from UT1 to TT.
                          7. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st September 2013
                          8. Addition of a UT1MinusUTC method which returns UT1 - UTC.
         PJN / 12-11-2014 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st October 2014
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2024
         PJN / 15-02-2015 1. Updated copyright details.
                          2. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st January 2015
                          3. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2024
                          4. Updated the CumulativeLeapSeconds table from http://maia.usno.navy.mil/ser7/tai-utc.dat to 1st July 2015
         PJN / 05-07-2015 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st April 2015
         PJN / 01-09-2015 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st July 2015
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2025
         PJN / 31-12-2015 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st October 2015
                          2. Verified the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds are up to date
         PJN / 09-03-2016 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st February 2016
                          2. Verified the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds are up to date
         PJN / 07-07-2016 1. Updated the CAADynamicalTime::CumulativeLeapSeconds method as taken from 
                          http://maia.usno.navy.mil/ser7/tai-utc.dat to include the leap second which will occur on 1 January 2017.
                          2. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st July 2016
                          3. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to July 2026
         PJN / 05-02-2017 1. Updated copyright details.
                          2. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st January 2017
                          3. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to Jan 2026
         PJN / 18-02-2017 1. Fixed a transcription error on the DeltaT value for 1 May 2015. The correct value is 67.8012 instead 
                          of 67.8011. Thanks to Luigi Candurro for reporting this error.
                          2. Fixed a number of transcription errors on the predicted DeltaT values from 2019.75 to 2025.75. Thanks 
                          to Luigi Candurro for reporting these errors.
                          3. CAADynamicalTime::TT2UTC is now implemented as TT2UT1 for date ranges prior to 1 January 1961 and 500
                          days after the last leap second (which is currently 1 January 2017). Also CAADynamicalTime::UTC2TT is 
                          now implemented as UT12TT for date ranges prior to 1 January 1961 and 500 days after the last leap 
                          second. These changes address problems where these two methods would end up using a constant offset
                          between UTC and TT for dates away from the current epoch. This problem was discovered while calculating 
                          rise, transit and set times for the Moon in B.C.E years. Thanks to Luigi Candurro for prompting this 
                          update.
         PJN / 19-02-2017 1. Fixed a bug in the CAADynamicalTime::UTC2TT & CAADynamicalTime::TT2UTC methods where the code would
                          incorrectly use BaseMJD instead of JD when determining if the date is in the valid range of UTC. Thanks
                          to Luigi Candurro for reporting this bug.
         PJN / 30-07-2017 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st June 2017
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to Jan 2026
         PJN / 20-01-2018 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st January 
                          2018
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to Jan 2026
         PJN / 22-09-2018 1. Updated the observed DeltaT values from http://toshi.nofs.navy.mil/ser7/deltat.data to 1st April 2018
         PJN / 03-01-2019 1. Updated copyright details.
                          2. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st October 2018
         PJN / 03-01-2019 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st January 2019
         PJN / 08-06-2019 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st April 2019
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to October 2027
         PJN / 23-06-2019 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st June 2019
         PJN / 18-08-2019 1. Fixed some further compiler warnings when using VC 2019 Preview v16.3.0 Preview 2.0
         PJN / 28-09-2019 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st September 2019

Copyright (c) 2003 - 2019 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////////////// Includes ///////////////////////////////////

#include "stdafx.h"
#include "AADynamicalTime.h"
#include "AADate.h"
#include <cassert>
#include <cstddef>
using namespace std;


////////////////////////////////// Macros / Defines ///////////////////////////

#ifdef _MSC_VER
#pragma warning(disable : 26446 26482)
#endif //#ifdef _MSC_VER

struct DeltaTValue
{
  double JD;
  double DeltaT;
};

const DeltaTValue g_DeltaTValues[] = 
{
//All the initial values are observed values from 1 February 1973 to 1 June 2017 as taken from http://maia.usno.navy.mil/ser7/deltat.data 
  { 2441714.5,  43.4724 },
  { 2441742.5,  43.5648 },
  { 2441773.5,  43.6737 },
  { 2441803.5,  43.7782 },
  { 2441834.5,  43.8763 },
  { 2441864.5,  43.9562 },
  { 2441895.5,  44.0315 },
  { 2441926.5,  44.1132 },
  { 2441956.5,  44.1982 },
  { 2441987.5,  44.2952 },
  { 2442017.5,  44.3936 },
  { 2442048.5,  44.4841 },
  { 2442079.5,  44.5646 },
  { 2442107.5,  44.6425 },
  { 2442138.5,  44.7386 },
  { 2442168.5,  44.8370 },
  { 2442199.5,  44.9302 },
  { 2442229.5,  44.9986 },
  { 2442260.5,  45.0584 },
  { 2442291.5,  45.1284 },
  { 2442321.5,  45.2064 },
  { 2442352.5,  45.2980 },
  { 2442382.5,  45.3897 },
  { 2442413.5,  45.4761 },
  { 2442444.5,  45.5633 },
  { 2442472.5,  45.6450 },
  { 2442503.5,  45.7375 },
  { 2442533.5,  45.8284 },
  { 2442564.5,  45.9133 },
  { 2442594.5,  45.9820 },
  { 2442625.5,  46.0408 },
  { 2442656.5,  46.1067 },
  { 2442686.5,  46.1825 },
  { 2442717.5,  46.2789 },
  { 2442747.5,  46.3713 },
  { 2442778.5,  46.4567 },
  { 2442809.5,  46.5445 },
  { 2442838.5,  46.6311 },
  { 2442869.5,  46.7302 },
  { 2442899.5,  46.8284 },
  { 2442930.5,  46.9247 },
  { 2442960.5,  46.9970 },
  { 2442991.5,  47.0709 },
  { 2443022.5,  47.1451 },
  { 2443052.5,  47.2362 },
  { 2443083.5,  47.3413 },
  { 2443113.5,  47.4319 },
  { 2443144.5,  47.5214 },
  { 2443175.5,  47.6049 },
  { 2443203.5,  47.6837 },
  { 2443234.5,  47.7781 },
  { 2443264.5,  47.8771 },
  { 2443295.5,  47.9687 },
  { 2443325.5,  48.0348 },
  { 2443356.5,  48.0942 },
  { 2443387.5,  48.1608 },
  { 2443417.5,  48.2460 },
  { 2443448.5,  48.3439 },
  { 2443478.5,  48.4355 },
  { 2443509.5,  48.5344 },
  { 2443540.5,  48.6325 },
  { 2443568.5,  48.7294 },
  { 2443599.5,  48.8365 },
  { 2443629.5,  48.9353 },
  { 2443660.5,  49.0319 },
  { 2443690.5,  49.1013 },
  { 2443721.5,  49.1591 },
  { 2443752.5,  49.2286 },
  { 2443782.5,  49.3070 },
  { 2443813.5,  49.4018 },
  { 2443843.5,  49.4945 },
  { 2443874.5,  49.5862 },
  { 2443905.5,  49.6805 },
  { 2443933.5,  49.7602 },
  { 2443964.5,  49.8556 },
  { 2443994.5,  49.9489 },
  { 2444025.5,  50.0347 },
  { 2444055.5,  50.1019 },
  { 2444086.5,  50.1622 },
  { 2444117.5,  50.2260 },
  { 2444147.5,  50.2968 },
  { 2444178.5,  50.3831 },
  { 2444208.5,  50.4599 },
  { 2444239.5,  50.5387 },
  { 2444270.5,  50.6161 },
  { 2444299.5,  50.6866 },
  { 2444330.5,  50.7658 },
  { 2444360.5,  50.8454 },
  { 2444391.5,  50.9187 },
  { 2444421.5,  50.9761 },
  { 2444452.5,  51.0278 },
  { 2444483.5,  51.0843 },
  { 2444513.5,  51.1538 },
  { 2444544.5,  51.2319 },
  { 2444574.5,  51.3063 },
  { 2444605.5,  51.3808 },
  { 2444636.5,  51.4526 },
  { 2444664.5,  51.5160 },
  { 2444695.5,  51.5985 },
  { 2444725.5,  51.6809 },
  { 2444756.5,  51.7573 },
  { 2444786.5,  51.8133 },
  { 2444817.5,  51.8532 },
  { 2444848.5,  51.9014 },
  { 2444878.5,  51.9603 },
  { 2444909.5,  52.0328 },
  { 2444939.5,  52.0985 },
  { 2444970.5,  52.1668 },
  { 2445001.5,  52.2316 },
  { 2445029.5,  52.2938 },
  { 2445060.5,  52.3680 },
  { 2445090.5,  52.4465 },
  { 2445121.5,  52.5180 },
  { 2445151.5,  52.5752 },
  { 2445182.5,  52.6178 },
  { 2445213.5,  52.6668 },
  { 2445243.5,  52.7340 },
  { 2445274.5,  52.8056 },
  { 2445304.5,  52.8792 },
  { 2445335.5,  52.9565 },
  { 2445366.5,  53.0445 },
  { 2445394.5,  53.1268 },
  { 2445425.5,  53.2197 },
  { 2445455.5,  53.3024 },
  { 2445486.5,  53.3747 },
  { 2445516.5,  53.4335 },
  { 2445547.5,  53.4778 },
  { 2445578.5,  53.5300 },
  { 2445608.5,  53.5845 },
  { 2445639.5,  53.6523 },
  { 2445669.5,  53.7256 },
  { 2445700.5,  53.7882 },
  { 2445731.5,  53.8367 },
  { 2445760.5,  53.8830 },
  { 2445791.5,  53.9443 },
  { 2445821.5,  54.0042 },
  { 2445852.5,  54.0536 },
  { 2445882.5,  54.0856 },
  { 2445913.5,  54.1084 },
  { 2445944.5,  54.1463 },
  { 2445974.5,  54.1914 },
  { 2446005.5,  54.2452 },
  { 2446035.5,  54.2958 },
  { 2446066.5,  54.3427 },
  { 2446097.5,  54.3911 },
  { 2446125.5,  54.4320 },
  { 2446156.5,  54.4898 },
  { 2446186.5,  54.5456 },
  { 2446217.5,  54.5977 },
  { 2446247.5,  54.6355 },
  { 2446278.5,  54.6532 },
  { 2446309.5,  54.6776 },
  { 2446339.5,  54.7174 },
  { 2446370.5,  54.7741 },
  { 2446400.5,  54.8253 },
  { 2446431.5,  54.8713 },
  { 2446462.5,  54.9161 },
  { 2446490.5,  54.9581 },
  { 2446521.5,  54.9997 },
  { 2446551.5,  55.0476 },
  { 2446582.5,  55.0912 },
  { 2446612.5,  55.1132 },
  { 2446643.5,  55.1328 },
  { 2446674.5,  55.1532 },
  { 2446704.5,  55.1898 },
  { 2446735.5,  55.2416 },
  { 2446765.5,  55.2838 },
  { 2446796.5,  55.3222 },
  { 2446827.5,  55.3613 },
  { 2446855.5,  55.4063 },
  { 2446886.5,  55.4629 },
  { 2446916.5,  55.5111 },
  { 2446947.5,  55.5524 },
  { 2446977.5,  55.5812 },
  { 2447008.5,  55.6004 },
  { 2447039.5,  55.6262 },
  { 2447069.5,  55.6656 },
  { 2447100.5,  55.7168 },
  { 2447130.5,  55.7698 },
  { 2447161.5,  55.8197 },
  { 2447192.5,  55.8615 },
  { 2447221.5,  55.9130 },
  { 2447252.5,  55.9663 },
  { 2447282.5,  56.0220 },
  { 2447313.5,  56.0700 },
  { 2447343.5,  56.0939 },
  { 2447374.5,  56.1105 },
  { 2447405.5,  56.1314 },
  { 2447435.5,  56.1611 },
  { 2447466.5,  56.2068 },
  { 2447496.5,  56.2583 },
  { 2447527.5,  56.3000 },
  { 2447558.5,  56.3399 },
  { 2447586.5,  56.3790 },
  { 2447617.5,  56.4283 },
  { 2447647.5,  56.4804 },
  { 2447678.5,  56.5352 },
  { 2447708.5,  56.5697 },
  { 2447739.5,  56.5983 },
  { 2447770.5,  56.6328 },
  { 2447800.5,  56.6739 },
  { 2447831.5,  56.7332 },
  { 2447861.5,  56.7972 },
  { 2447892.5,  56.8553 },
  { 2447923.5,  56.9111 },
  { 2447951.5,  56.9755 },
  { 2447982.5,  57.0471 },
  { 2448012.5,  57.1136 },
  { 2448043.5,  57.1738 },
  { 2448073.5,  57.2226 },
  { 2448104.5,  57.2597 },
  { 2448135.5,  57.3073 },
  { 2448165.5,  57.3643 },
  { 2448196.5,  57.4334 },
  { 2448226.5,  57.5016 },
  { 2448257.5,  57.5653 },
  { 2448288.5,  57.6333 },
  { 2448316.5,  57.6973 },
  { 2448347.5,  57.7711 },
  { 2448377.5,  57.8407 },
  { 2448408.5,  57.9058 },
  { 2448438.5,  57.9576 },
  { 2448469.5,  57.9975 },
  { 2448500.5,  58.0426 },
  { 2448530.5,  58.1043 },
  { 2448561.5,  58.1679 },
  { 2448591.5,  58.2389 },
  { 2448622.5,  58.3092 },
  { 2448653.5,  58.3833 },
  { 2448682.5,  58.4537 },
  { 2448713.5,  58.5401 },
  { 2448743.5,  58.6228 },
  { 2448774.5,  58.6917 },
  { 2448804.5,  58.7410 },
  { 2448835.5,  58.7836 },
  { 2448866.5,  58.8406 },
  { 2448896.5,  58.8986 },
  { 2448927.5,  58.9714 },
  { 2448957.5,  59.0438 },
  { 2448988.5,  59.1218 },
  { 2449019.5,  59.2003 },
  { 2449047.5,  59.2747 },
  { 2449078.5,  59.3574 },
  { 2449108.5,  59.4434 },
  { 2449139.5,  59.5242 },
  { 2449169.5,  59.5850 },
  { 2449200.5,  59.6344 },
  { 2449231.5,  59.6928 },
  { 2449261.5,  59.7588 },
  { 2449292.5,  59.8386 },
  { 2449322.5,  59.9111 },
  { 2449353.5,  59.9845 },
  { 2449384.5,  60.0564 },
  { 2449412.5,  60.1231 },
  { 2449443.5,  60.2042 },
  { 2449473.5,  60.2804 },
  { 2449504.5,  60.3530 },
  { 2449534.5,  60.4012 },
  { 2449565.5,  60.4440 },
  { 2449596.5,  60.4900 },
  { 2449626.5,  60.5578 },
  { 2449657.5,  60.6324 },
  { 2449687.5,  60.7059 },
  { 2449718.5,  60.7853 },
  { 2449749.5,  60.8664 },
  { 2449777.5,  60.9387 },
  { 2449808.5,  61.0277 },
  { 2449838.5,  61.1103 },
  { 2449869.5,  61.1870 },
  { 2449899.5,  61.2454 },
  { 2449930.5,  61.2881 },
  { 2449961.5,  61.3378 },
  { 2449991.5,  61.4036 },
  { 2450022.5,  61.4760 },
  { 2450052.5,  61.5525 },
  { 2450083.5,  61.6287 },
  { 2450114.5,  61.6846 },
  { 2450143.5,  61.7433 },
  { 2450174.5,  61.8132 },
  { 2450204.5,  61.8823 },
  { 2450235.5,  61.9497 },
  { 2450265.5,  61.9969 },
  { 2450296.5,  62.0343 },
  { 2450327.5,  62.0714 },
  { 2450357.5,  62.1202 },
  { 2450388.5,  62.1810 },
  { 2450418.5,  62.2382 },
  { 2450449.5,  62.2950 },
  { 2450480.5,  62.3506 },
  { 2450508.5,  62.3995 },
  { 2450539.5,  62.4754 },
  { 2450569.5,  62.5463 },
  { 2450600.5,  62.6136 },
  { 2450630.5,  62.6571 },
  { 2450661.5,  62.6942 },
  { 2450692.5,  62.7383 },
  { 2450722.5,  62.7926 },
  { 2450753.5,  62.8567 },
  { 2450783.5,  62.9146 },
  { 2450814.5,  62.9659 },
  { 2450845.5,  63.0217 },
  { 2450873.5,  63.0807 },
  { 2450904.5,  63.1462 },
  { 2450934.5,  63.2053 },
  { 2450965.5,  63.2599 },
  { 2450995.5,  63.2844 },
  { 2451026.5,  63.2961 },
  { 2451057.5,  63.3126 },
  { 2451087.5,  63.3422 },
  { 2451118.5,  63.3871 },
  { 2451148.5,  63.4339 },
  { 2451179.5,  63.4673 },
  { 2451210.5,  63.4979 },
  { 2451238.5,  63.5319 },
  { 2451269.5,  63.5679 },
  { 2451299.5,  63.6104 },
  { 2451330.5,  63.6444 },
  { 2451360.5,  63.6642 },
  { 2451391.5,  63.6739 },
  { 2451422.5,  63.6926 },
  { 2451452.5,  63.7147 },
  { 2451483.5,  63.7518 },
  { 2451513.5,  63.7927 },
  { 2451544.5,  63.8285 },
  { 2451575.5,  63.8557 },
  { 2451604.5,  63.8804 },
  { 2451635.5,  63.9075 },
  { 2451665.5,  63.9393 },
  { 2451696.5,  63.9691 },
  { 2451726.5,  63.9799 },
  { 2451757.5,  63.9833 },
  { 2451788.5,  63.9938 },
  { 2451818.5,  64.0093 },
  { 2451849.5,  64.0400 },
  { 2451879.5,  64.0670 },
  { 2451910.5,  64.0908 },
  { 2451941.5,  64.1068 },
  { 2451969.5,  64.1282 },
  { 2452000.5,  64.1584 },
  { 2452030.5,  64.1833 },
  { 2452061.5,  64.2094 },
  { 2452091.5,  64.2117 },
  { 2452122.5,  64.2073 },
  { 2452153.5,  64.2116 },
  { 2452183.5,  64.2223 },
  { 2452214.5,  64.2500 },
  { 2452244.5,  64.2761 },
  { 2452275.5,  64.2998 },
  { 2452306.5,  64.3192 },
  { 2452334.5,  64.3450 },
  { 2452365.5,  64.3735 },
  { 2452395.5,  64.3943 },
  { 2452426.5,  64.4151 },
  { 2452456.5,  64.4132 },
  { 2452487.5,  64.4118 },
  { 2452518.5,  64.4097 },
  { 2452548.5,  64.4168 },
  { 2452579.5,  64.4329 },
  { 2452609.5,  64.4511 },
  { 2452640.5,  64.4734 },
  { 2452671.5,  64.4893 },
  { 2452699.5,  64.5053 },
  { 2452730.5,  64.5269 },
  { 2452760.5,  64.5471 },
  { 2452791.5,  64.5597 },
  { 2452821.5,  64.5512 },
  { 2452852.5,  64.5371 },
  { 2452883.5,  64.5359 },
  { 2452913.5,  64.5415 },
  { 2452944.5,  64.5544 },
  { 2452974.5,  64.5654 },
  { 2453005.5,  64.5736 },
  { 2453036.5,  64.5891 },
  { 2453065.5,  64.6015 },
  { 2453096.5,  64.6176 },
  { 2453126.5,  64.6374 },
  { 2453157.5,  64.6549 },
  { 2453187.5,  64.6530 },
  { 2453218.5,  64.6379 },
  { 2453249.5,  64.6372 },
  { 2453279.5,  64.6400 },
  { 2453310.5,  64.6543 },
  { 2453340.5,  64.6723 },
  { 2453371.5,  64.6876 },
  { 2453402.5,  64.7052 },
  { 2453430.5,  64.7313 },
  { 2453461.5,  64.7575 },
  { 2453491.5,  64.7811 },
  { 2453522.5,  64.8001 },
  { 2453552.5,  64.7995 },
  { 2453583.5,  64.7876 },
  { 2453614.5,  64.7831 },
  { 2453644.5,  64.7921 },
  { 2453675.5,  64.8096 },
  { 2453705.5,  64.8311 },
  { 2453736.5,  64.8452 },
  { 2453767.5,  64.8597 },
  { 2453795.5,  64.8850 },
  { 2453826.5,  64.9175 },
  { 2453856.5,  64.9480 },
  { 2453887.5,  64.9794 },
  { 2453917.5,  64.9895 },
  { 2453948.5,  65.0028 },
  { 2453979.5,  65.0138 },
  { 2454009.5,  65.0371 },
  { 2454040.5,  65.0773 },
  { 2454070.5,  65.1122 },
  { 2454101.5,  65.1464 },
  { 2454132.5,  65.1833 },
  { 2454160.5,  65.2145 },
  { 2454191.5,  65.2494 },
  { 2454221.5,  65.2921 },
  { 2454252.5,  65.3279 },
  { 2454282.5,  65.3413 },
  { 2454313.5,  65.3452 },
  { 2454344.5,  65.3496 },
  { 2454374.5,  65.3711 },
  { 2454405.5,  65.3972 },
  { 2454435.5,  65.4296 },
  { 2454466.5,  65.4573 },
  { 2454497.5,  65.4868 },
  { 2454526.5,  65.5152 },
  { 2454557.5,  65.5450 },
  { 2454587.5,  65.5781 },
  { 2454618.5,  65.6127 },
  { 2454648.5,  65.6288 },
  { 2454679.5,  65.6370 },
  { 2454710.5,  65.6493 },
  { 2454740.5,  65.6760 },
  { 2454771.5,  65.7097 },
  { 2454801.5,  65.7461 },
  { 2454832.5,  65.7768 },
  { 2454863.5,  65.8025 },
  { 2454891.5,  65.8237 },
  { 2454922.5,  65.8595 },
  { 2454952.5,  65.8973 },
  { 2454983.5,  65.9323 },
  { 2455013.5,  65.9509 },
  { 2455044.5,  65.9534 },
  { 2455075.5,  65.9628 },
  { 2455105.5,  65.9839 },
  { 2455136.5,  66.0147 },
  { 2455166.5,  66.0420 },
  { 2455197.5,  66.0699 },
  { 2455228.5,  66.0961 },
  { 2455256.5,  66.1310 },
  { 2455287.5,  66.1683 },
  { 2455317.5,  66.2072 },
  { 2455348.5,  66.2356 },
  { 2455378.5,  66.2409 },
  { 2455409.5,  66.2335 },
  { 2455440.5,  66.2349 },
  { 2455470.5,  66.2441 },
  { 2455501.5,  66.2751 },
  { 2455531.5,  66.3054 },
  { 2455562.5,  66.3246 },
  { 2455593.5,  66.3406 },
  { 2455621.5,  66.3624 },
  { 2455652.5,  66.3957 },
  { 2455682.5,  66.4289 },
  { 2455713.5,  66.4619 },
  { 2455743.5,  66.4749 },
  { 2455774.5,  66.4751 },
  { 2455805.5,  66.4829 },
  { 2455835.5,  66.5056 },
  { 2455866.5,  66.5383 },
  { 2455896.5,  66.5706 },
  { 2455927.5,  66.6030 },
  { 2455958.5,  66.6340 },
  { 2455987.5,  66.6569 },
  { 2456018.5,  66.6925 }, //1 April 2012
  { 2456048.5,  66.7289 },
  { 2456079.5,  66.7579 },
  { 2456109.5,  66.7708 },
  { 2456140.5,  66.7740 },
  { 2456171.5,  66.7846 },
  { 2456201.5,  66.8103 },
  { 2456232.5,  66.8400 },
  { 2456262.5,  66.8779 },
  { 2456293.5,  66.9069 }, //1 January 2013
  { 2456324.5,  66.9443 }, //1 Februrary 2013
  { 2456352.5,  66.9763 }, //1 March 2013
  { 2456383.5,  67.0258 }, //1 April 2013
  { 2456413.5,  67.0716 }, //1 May 2013
  { 2456444.5,  67.1100 }, //1 June 2013
  { 2456474.5,  67.1266 }, //1 July 2013
  { 2456505.5,  67.1331 }, //1 August 2013
  { 2456536.5,  67.1458 }, //1 September 2013
  { 2456566.5,  67.1717 }, //1 October 2013
  { 2456597.5,  67.2091 }, //1 November 2013
  { 2456627.5,  67.2460 }, //1 December 2013
  { 2456658.5,  67.2810 }, //1 January 2014
  { 2456689.5,  67.3136 }, //1 February 2014
  { 2456717.5,  67.3457 }, //1 March 2014
  { 2456748.5,  67.3890 }, //1 April 2014
  { 2456778.5,  67.4318 }, //1 May 2014
  { 2456809.5,  67.4666 }, //1 June 2014
  { 2456839.5,  67.4858 }, //1 July 2014
  { 2456870.5,  67.4989 }, //1 August 2014
  { 2456901.5,  67.5111 }, //1 September 2014
  { 2456931.5,  67.5353 }, //1 October 2014
  { 2456962.5,  67.5711 }, //1 November 2014
  { 2456992.5,  67.6070 }, //1 December 2014
  { 2457023.5,  67.6439 }, //1 January 2015
  { 2457054.5,  67.6765 }, //1 February 2015
  { 2457082.5,  67.7117 }, //1 March 2015
  { 2457113.5,  67.7591 }, //1 April 2015
  { 2457143.5,  67.8012 }, //1 May 2015
  { 2457174.5,  67.8402 }, //1 June 2015
  { 2457204.5,  67.8606 }, //1 July 2015
  { 2457235.5,  67.8822 }, //1 August 2015
  { 2457266.5,  67.9120 }, //1 September 2015
  { 2457296.5,  67.9546 }, //1 October 2015
  { 2457327.5,  68.0055 }, //1 November 2015
  { 2457357.5,  68.0514 }, //1 December 2015
  { 2457388.5,  68.1024 }, //1 January 2016
  { 2457419.5,  68.1577 }, //1 February 2016
  { 2457448.5,  68.2044 }, //1 March 2016
  { 2457479.5,  68.2665 }, //1 April 2016
  { 2457509.5,  68.3188 }, //1 May 2016
  { 2457540.5,  68.3703 }, //1 June 2016
  { 2457570.5,  68.3964 }, //1 July 2016
  { 2457601.5,  68.4094 }, //1 August 2016
  { 2457632.5,  68.4305 }, //1 September 2016
  { 2457662.5,  68.4630 }, //1 October 2016
  { 2457693.5,  68.5078 }, //1 November 2016
  { 2457723.5,  68.5537 }, //1 December 2016
  { 2457754.5,  68.5928 }, //1 January 2017
  { 2457785.5,  68.6298 }, //1 February 2017
  { 2457813.5,  68.6671 }, //1 March 2017
  { 2457844.5,  68.7135 }, //1 April 2017
  { 2457874.5,  68.7623 }, //1 May 2017
  { 2457905.5,  68.8033 }, //1 June 2017
  { 2457935.5,  68.8245 }, //1 July 2017
  { 2457966.5,  68.8373 }, //1 August 2017
  { 2457997.5,  68.8477 }, //1 September 2017
  { 2458027.5,  68.8689 }, //1 October 2017
  { 2458058.5,  68.9006 }, //1 November 2017
  { 2458088.5,  68.9355 }, //1 December 2017
  { 2458119.5,  68.9676 }, //1 January 2018
  { 2458150.5,  68.9875 }, //1 February 2018
  { 2458178.5,  69.0175 }, //1 March 2018
  { 2458209.5,  69.0499 }, //1 April 2018
  { 2458239.5,  69.0823 }, //1 May 2018
  { 2458270.5,  69.1070 }, //1 June 2018
  { 2458300.5,  69.1134 }, //1 July 2018
  { 2458331.5,  69.1142 }, //1 August 2018
  { 2458362.5,  69.1207 }, //1 September 2018
  { 2458392.5,  69.1356 }, //1 October 2018
  { 2458423.5,  69.1646 }, //1 November 2018
  { 2458453.5,  69.1964 }, //1 December 2018
  { 2458484.5,  69.2201 }, //1 January 2019
  { 2458515.5,  69.2452 }, //1 February 2019
  { 2458543.5,  69.2733 }, //1 March 2019
  { 2458574.5,  69.3032 }, //1 April 2019
  { 2458604.5,  69.3326 }, //1 May 2019
  { 2458635.5,  69.3540 }, //1 June 2019
  { 2458665.5,  69.3582 }, //1 July 2019
  { 2458696.5,  69.3442 }, //1 August 2019
    { 2458727.50, 69.337633 }, //01 September 2019, UT1-UTC=-0.1536329, Accumulated Leap Seconds=37
    { 2458728.50, 69.338265 }, //02 September 2019, UT1-UTC=-0.1542651, Accumulated Leap Seconds=37
    { 2458729.50, 69.338814 }, //03 September 2019, UT1-UTC=-0.1548143, Accumulated Leap Seconds=37
    { 2458730.50, 69.339264 }, //04 September 2019, UT1-UTC=-0.1552635, Accumulated Leap Seconds=37
    { 2458731.50, 69.339524 }, //05 September 2019, UT1-UTC=-0.1555240, Accumulated Leap Seconds=37
    { 2458732.50, 69.339542 }, //06 September 2019, UT1-UTC=-0.1555421, Accumulated Leap Seconds=37
    { 2458733.50, 69.339368 }, //07 September 2019, UT1-UTC=-0.1553677, Accumulated Leap Seconds=37
    { 2458734.50, 69.339106 }, //08 September 2019, UT1-UTC=-0.1551056, Accumulated Leap Seconds=37
    { 2458735.50, 69.338717 }, //09 September 2019, UT1-UTC=-0.1547169, Accumulated Leap Seconds=37
    { 2458736.50, 69.338301 }, //10 September 2019, UT1-UTC=-0.1543014, Accumulated Leap Seconds=37
    { 2458737.50, 69.337939 }, //11 September 2019, UT1-UTC=-0.1539390, Accumulated Leap Seconds=37
    { 2458738.50, 69.337596 }, //12 September 2019, UT1-UTC=-0.1535956, Accumulated Leap Seconds=37
    { 2458739.50, 69.337369 }, //13 September 2019, UT1-UTC=-0.1533692, Accumulated Leap Seconds=37
    { 2458740.50, 69.337299 }, //14 September 2019, UT1-UTC=-0.1532988, Accumulated Leap Seconds=37
    { 2458741.50, 69.337325 }, //15 September 2019, UT1-UTC=-0.1533251, Accumulated Leap Seconds=37
    { 2458742.50, 69.337337 }, //16 September 2019, UT1-UTC=-0.1533367, Accumulated Leap Seconds=37
    { 2458743.50, 69.337324 }, //17 September 2019, UT1-UTC=-0.1533243, Accumulated Leap Seconds=37
    { 2458744.50, 69.337282 }, //18 September 2019, UT1-UTC=-0.1532823, Accumulated Leap Seconds=37
    { 2458745.50, 69.337151 }, //19 September 2019, UT1-UTC=-0.1531511, Accumulated Leap Seconds=37
    { 2458746.50, 69.336911 }, //20 September 2019, UT1-UTC=-0.1529114, Accumulated Leap Seconds=37
    { 2458747.50, 69.336552 }, //21 September 2019, UT1-UTC=-0.1525524, Accumulated Leap Seconds=37
    { 2458748.50, 69.336108 }, //22 September 2019, UT1-UTC=-0.1521081, Accumulated Leap Seconds=37
    { 2458749.50, 69.335646 }, //23 September 2019, UT1-UTC=-0.1516463, Accumulated Leap Seconds=37
    { 2458750.50, 69.335209 }, //24 September 2019, UT1-UTC=-0.1512088, Accumulated Leap Seconds=37
    { 2458751.50, 69.334915 }, //25 September 2019, UT1-UTC=-0.1509148, Accumulated Leap Seconds=37
    { 2458752.50, 69.334903 }, //26 September 2019, UT1-UTC=-0.1509029, Accumulated Leap Seconds=37
    { 2458753.50, 69.335215 }, //27 September 2019, UT1-UTC=-0.1512152, Accumulated Leap Seconds=37
    { 2458754.50, 69.335802 }, //28 September 2019, UT1-UTC=-0.1518024, Accumulated Leap Seconds=37
    { 2458755.50, 69.336506 }, //29 September 2019, UT1-UTC=-0.1525061, Accumulated Leap Seconds=37
    { 2458756.50, 69.337197 }, //30 September 2019, UT1-UTC=-0.1531969, Accumulated Leap Seconds=37
    { 2458757.50, 69.337742 }, //01 October 2019, UT1-UTC=-0.1537424, Accumulated Leap Seconds=37
    { 2458758.50, 69.337982 }, //02 October 2019, UT1-UTC=-0.1539819, Accumulated Leap Seconds=37
    { 2458759.50, 69.337930 }, //03 October 2019, UT1-UTC=-0.1539302, Accumulated Leap Seconds=37
    { 2458760.50, 69.337687 }, //04 October 2019, UT1-UTC=-0.1536867, Accumulated Leap Seconds=37
    { 2458761.50, 69.337299 }, //05 October 2019, UT1-UTC=-0.1532989, Accumulated Leap Seconds=37
    { 2458762.50, 69.336834 }, //06 October 2019, UT1-UTC=-0.1528341, Accumulated Leap Seconds=37
    { 2458763.50, 69.336374 }, //07 October 2019, UT1-UTC=-0.1523745, Accumulated Leap Seconds=37
    { 2458764.50, 69.335972 }, //08 October 2019, UT1-UTC=-0.1519718, Accumulated Leap Seconds=37
    { 2458765.50, 69.335646 }, //09 October 2019, UT1-UTC=-0.1516461, Accumulated Leap Seconds=37
    { 2458766.50, 69.335433 }, //10 October 2019, UT1-UTC=-0.1514326, Accumulated Leap Seconds=37
    { 2458767.50, 69.335374 }, //11 October 2019, UT1-UTC=-0.1513740, Accumulated Leap Seconds=37
    { 2458768.50, 69.335478 }, //12 October 2019, UT1-UTC=-0.1514780, Accumulated Leap Seconds=37
    { 2458769.50, 69.335673 }, //13 October 2019, UT1-UTC=-0.1516730, Accumulated Leap Seconds=37
    { 2458770.50, 69.335935 }, //14 October 2019, UT1-UTC=-0.1519347, Accumulated Leap Seconds=37
    { 2458771.50, 69.336240 }, //15 October 2019, UT1-UTC=-0.1522400, Accumulated Leap Seconds=37
    { 2458772.50, 69.336573 }, //16 October 2019, UT1-UTC=-0.1525734, Accumulated Leap Seconds=37
    { 2458773.50, 69.336884 }, //17 October 2019, UT1-UTC=-0.1528842, Accumulated Leap Seconds=37
    { 2458774.50, 69.337135 }, //18 October 2019, UT1-UTC=-0.1531350, Accumulated Leap Seconds=37
    { 2458775.50, 69.337321 }, //19 October 2019, UT1-UTC=-0.1533206, Accumulated Leap Seconds=37
    { 2458776.50, 69.337419 }, //20 October 2019, UT1-UTC=-0.1534187, Accumulated Leap Seconds=37
    { 2458777.50, 69.337490 }, //21 October 2019, UT1-UTC=-0.1534904, Accumulated Leap Seconds=37
    { 2458778.50, 69.337599 }, //22 October 2019, UT1-UTC=-0.1535986, Accumulated Leap Seconds=37
    { 2458779.50, 69.337818 }, //23 October 2019, UT1-UTC=-0.1538178, Accumulated Leap Seconds=37
    { 2458780.50, 69.338240 }, //24 October 2019, UT1-UTC=-0.1542404, Accumulated Leap Seconds=37
    { 2458781.50, 69.338915 }, //25 October 2019, UT1-UTC=-0.1549155, Accumulated Leap Seconds=37
    { 2458782.50, 69.339816 }, //26 October 2019, UT1-UTC=-0.1558157, Accumulated Leap Seconds=37
    { 2458783.50, 69.340812 }, //27 October 2019, UT1-UTC=-0.1568124, Accumulated Leap Seconds=37
    { 2458784.50, 69.341742 }, //28 October 2019, UT1-UTC=-0.1577418, Accumulated Leap Seconds=37
    { 2458785.50, 69.342452 }, //29 October 2019, UT1-UTC=-0.1584521, Accumulated Leap Seconds=37
    { 2458786.50, 69.342919 }, //30 October 2019, UT1-UTC=-0.1589190, Accumulated Leap Seconds=37
    { 2458787.50, 69.343150 }, //31 October 2019, UT1-UTC=-0.1591500, Accumulated Leap Seconds=37
    { 2458788.50, 69.343219 }, //01 November 2019, UT1-UTC=-0.1592191, Accumulated Leap Seconds=37
    { 2458789.50, 69.343203 }, //02 November 2019, UT1-UTC=-0.1592033, Accumulated Leap Seconds=37
    { 2458790.50, 69.343177 }, //03 November 2019, UT1-UTC=-0.1591771, Accumulated Leap Seconds=37
    { 2458791.50, 69.343217 }, //04 November 2019, UT1-UTC=-0.1592174, Accumulated Leap Seconds=37
    { 2458792.50, 69.343284 }, //05 November 2019, UT1-UTC=-0.1592844, Accumulated Leap Seconds=37
    { 2458793.50, 69.343451 }, //06 November 2019, UT1-UTC=-0.1594509, Accumulated Leap Seconds=37
    { 2458794.50, 69.343776 }, //07 November 2019, UT1-UTC=-0.1597762, Accumulated Leap Seconds=37
    { 2458795.50, 69.344251 }, //08 November 2019, UT1-UTC=-0.1602505, Accumulated Leap Seconds=37
    { 2458796.50, 69.344864 }, //09 November 2019, UT1-UTC=-0.1608641, Accumulated Leap Seconds=37
    { 2458797.50, 69.345559 }, //10 November 2019, UT1-UTC=-0.1615591, Accumulated Leap Seconds=37
    { 2458798.50, 69.346267 }, //11 November 2019, UT1-UTC=-0.1622667, Accumulated Leap Seconds=37
    { 2458799.50, 69.346901 }, //12 November 2019, UT1-UTC=-0.1629007, Accumulated Leap Seconds=37
    { 2458800.50, 69.347435 }, //13 November 2019, UT1-UTC=-0.1634352, Accumulated Leap Seconds=37
    { 2458801.50, 69.347865 }, //14 November 2019, UT1-UTC=-0.1638651, Accumulated Leap Seconds=37
    { 2458802.50, 69.348175 }, //15 November 2019, UT1-UTC=-0.1641749, Accumulated Leap Seconds=37
    { 2458803.50, 69.348388 }, //16 November 2019, UT1-UTC=-0.1643885, Accumulated Leap Seconds=37
    { 2458804.50, 69.348470 }, //17 November 2019, UT1-UTC=-0.1644699, Accumulated Leap Seconds=37
    { 2458805.50, 69.348518 }, //18 November 2019, UT1-UTC=-0.1645181, Accumulated Leap Seconds=37
    { 2458806.50, 69.348658 }, //19 November 2019, UT1-UTC=-0.1646584, Accumulated Leap Seconds=37
    { 2458807.50, 69.348964 }, //20 November 2019, UT1-UTC=-0.1649642, Accumulated Leap Seconds=37
    { 2458808.50, 69.349553 }, //21 November 2019, UT1-UTC=-0.1655534, Accumulated Leap Seconds=37
    { 2458809.50, 69.350340 }, //22 November 2019, UT1-UTC=-0.1663395, Accumulated Leap Seconds=37
    { 2458810.50, 69.351211 }, //23 November 2019, UT1-UTC=-0.1672110, Accumulated Leap Seconds=37
    { 2458811.50, 69.352096 }, //24 November 2019, UT1-UTC=-0.1680955, Accumulated Leap Seconds=37
    { 2458812.50, 69.352901 }, //25 November 2019, UT1-UTC=-0.1689013, Accumulated Leap Seconds=37
    { 2458813.50, 69.353475 }, //26 November 2019, UT1-UTC=-0.1694753, Accumulated Leap Seconds=37
    { 2458814.50, 69.353787 }, //27 November 2019, UT1-UTC=-0.1697867, Accumulated Leap Seconds=37
    { 2458815.50, 69.353946 }, //28 November 2019, UT1-UTC=-0.1699463, Accumulated Leap Seconds=37
    { 2458816.50, 69.354020 }, //29 November 2019, UT1-UTC=-0.1700196, Accumulated Leap Seconds=37
    { 2458817.50, 69.354032 }, //30 November 2019, UT1-UTC=-0.1700320, Accumulated Leap Seconds=37
    { 2458818.50, 69.354014 }, //01 December 2019, UT1-UTC=-0.1700144, Accumulated Leap Seconds=37
    { 2458819.50, 69.354067 }, //02 December 2019, UT1-UTC=-0.1700672, Accumulated Leap Seconds=37
    { 2458820.50, 69.354205 }, //03 December 2019, UT1-UTC=-0.1702051, Accumulated Leap Seconds=37
    { 2458821.50, 69.354424 }, //04 December 2019, UT1-UTC=-0.1704239, Accumulated Leap Seconds=37
    { 2458822.50, 69.354692 }, //05 December 2019, UT1-UTC=-0.1706918, Accumulated Leap Seconds=37
    { 2458823.50, 69.355027 }, //06 December 2019, UT1-UTC=-0.1710269, Accumulated Leap Seconds=37
    { 2458824.50, 69.355423 }, //07 December 2019, UT1-UTC=-0.1714231, Accumulated Leap Seconds=37
    { 2458825.50, 69.355864 }, //08 December 2019, UT1-UTC=-0.1718637, Accumulated Leap Seconds=37
    { 2458826.50, 69.356268 }, //09 December 2019, UT1-UTC=-0.1722681, Accumulated Leap Seconds=37
    { 2458827.50, 69.356535 }, //10 December 2019, UT1-UTC=-0.1725355, Accumulated Leap Seconds=37
    { 2458828.50, 69.356624 }, //11 December 2019, UT1-UTC=-0.1726237, Accumulated Leap Seconds=37
    { 2458829.50, 69.356599 }, //12 December 2019, UT1-UTC=-0.1725987, Accumulated Leap Seconds=37
    { 2458830.50, 69.356460 }, //13 December 2019, UT1-UTC=-0.1724603, Accumulated Leap Seconds=37
    { 2458831.50, 69.356275 }, //14 December 2019, UT1-UTC=-0.1722749, Accumulated Leap Seconds=37
    { 2458832.50, 69.356102 }, //15 December 2019, UT1-UTC=-0.1721025, Accumulated Leap Seconds=37
    { 2458833.50, 69.355998 }, //16 December 2019, UT1-UTC=-0.1719983, Accumulated Leap Seconds=37
    { 2458834.50, 69.356034 }, //17 December 2019, UT1-UTC=-0.1720340, Accumulated Leap Seconds=37
    { 2458835.50, 69.356248 }, //18 December 2019, UT1-UTC=-0.1722479, Accumulated Leap Seconds=37
    { 2458836.50, 69.356718 }, //19 December 2019, UT1-UTC=-0.1727185, Accumulated Leap Seconds=37
    { 2458837.50, 69.357355 }, //20 December 2019, UT1-UTC=-0.1733550, Accumulated Leap Seconds=37
    { 2458838.50, 69.358038 }, //21 December 2019, UT1-UTC=-0.1740384, Accumulated Leap Seconds=37
    { 2458839.50, 69.358719 }, //22 December 2019, UT1-UTC=-0.1747194, Accumulated Leap Seconds=37
    { 2458840.50, 69.359312 }, //23 December 2019, UT1-UTC=-0.1753123, Accumulated Leap Seconds=37
    { 2458841.50, 69.359757 }, //24 December 2019, UT1-UTC=-0.1757574, Accumulated Leap Seconds=37
    { 2458842.50, 69.360024 }, //25 December 2019, UT1-UTC=-0.1760245, Accumulated Leap Seconds=37
    { 2458843.50, 69.360146 }, //26 December 2019, UT1-UTC=-0.1761465, Accumulated Leap Seconds=37
    { 2458844.50, 69.360209 }, //27 December 2019, UT1-UTC=-0.1762087, Accumulated Leap Seconds=37
    { 2458845.50, 69.360251 }, //28 December 2019, UT1-UTC=-0.1762508, Accumulated Leap Seconds=37
    { 2458846.50, 69.360326 }, //29 December 2019, UT1-UTC=-0.1763259, Accumulated Leap Seconds=37
    { 2458847.50, 69.360492 }, //30 December 2019, UT1-UTC=-0.1764922, Accumulated Leap Seconds=37
    { 2458848.50, 69.360769 }, //31 December 2019, UT1-UTC=-0.1767688, Accumulated Leap Seconds=37
    { 2458849.50, 69.361155 }, //01 January 2020, UT1-UTC=-0.1771554, Accumulated Leap Seconds=37
    { 2458850.50, 69.361627 }, //02 January 2020, UT1-UTC=-0.1776274, Accumulated Leap Seconds=37
    { 2458851.50, 69.362115 }, //03 January 2020, UT1-UTC=-0.1781148, Accumulated Leap Seconds=37
    { 2458852.50, 69.362589 }, //04 January 2020, UT1-UTC=-0.1785885, Accumulated Leap Seconds=37
    { 2458853.50, 69.363082 }, //05 January 2020, UT1-UTC=-0.1790818, Accumulated Leap Seconds=37
    { 2458854.50, 69.363428 }, //06 January 2020, UT1-UTC=-0.1794276, Accumulated Leap Seconds=37
    { 2458855.50, 69.363551 }, //07 January 2020, UT1-UTC=-0.1795514, Accumulated Leap Seconds=37
    { 2458856.50, 69.363530 }, //08 January 2020, UT1-UTC=-0.1795302, Accumulated Leap Seconds=37
    { 2458857.50, 69.363375 }, //09 January 2020, UT1-UTC=-0.1793750, Accumulated Leap Seconds=37
    { 2458858.50, 69.363117 }, //10 January 2020, UT1-UTC=-0.1791174, Accumulated Leap Seconds=37
    { 2458859.50, 69.362879 }, //11 January 2020, UT1-UTC=-0.1788786, Accumulated Leap Seconds=37
    { 2458860.50, 69.362790 }, //12 January 2020, UT1-UTC=-0.1787902, Accumulated Leap Seconds=37
    { 2458861.50, 69.362938 }, //13 January 2020, UT1-UTC=-0.1789375, Accumulated Leap Seconds=37
    { 2458862.50, 69.363328 }, //14 January 2020, UT1-UTC=-0.1793277, Accumulated Leap Seconds=37
    { 2458863.50, 69.363964 }, //15 January 2020, UT1-UTC=-0.1799645, Accumulated Leap Seconds=37
    { 2458864.50, 69.364793 }, //16 January 2020, UT1-UTC=-0.1807929, Accumulated Leap Seconds=37
    { 2458865.50, 69.365743 }, //17 January 2020, UT1-UTC=-0.1817427, Accumulated Leap Seconds=37
    { 2458866.50, 69.366785 }, //18 January 2020, UT1-UTC=-0.1827849, Accumulated Leap Seconds=37
    { 2458867.50, 69.367757 }, //19 January 2020, UT1-UTC=-0.1837574, Accumulated Leap Seconds=37
    { 2458868.50, 69.368602 }, //20 January 2020, UT1-UTC=-0.1846024, Accumulated Leap Seconds=37
    { 2458869.50, 69.369284 }, //21 January 2020, UT1-UTC=-0.1852838, Accumulated Leap Seconds=37
    { 2458870.50, 69.369801 }, //22 January 2020, UT1-UTC=-0.1858012, Accumulated Leap Seconds=37
    { 2458871.50, 69.370164 }, //23 January 2020, UT1-UTC=-0.1861639, Accumulated Leap Seconds=37
    { 2458872.50, 69.370469 }, //24 January 2020, UT1-UTC=-0.1864690, Accumulated Leap Seconds=37
    { 2458873.50, 69.370784 }, //25 January 2020, UT1-UTC=-0.1867835, Accumulated Leap Seconds=37
    { 2458874.50, 69.371192 }, //26 January 2020, UT1-UTC=-0.1871917, Accumulated Leap Seconds=37
    { 2458875.50, 69.371708 }, //27 January 2020, UT1-UTC=-0.1877080, Accumulated Leap Seconds=37
    { 2458876.50, 69.372327 }, //28 January 2020, UT1-UTC=-0.1883268, Accumulated Leap Seconds=37
    { 2458877.50, 69.373052 }, //29 January 2020, UT1-UTC=-0.1890522, Accumulated Leap Seconds=37
    { 2458878.50, 69.373813 }, //30 January 2020, UT1-UTC=-0.1898134, Accumulated Leap Seconds=37
    { 2458879.50, 69.374520 }, //31 January 2020, UT1-UTC=-0.1905200, Accumulated Leap Seconds=37
    { 2458880.50, 69.375170 }, //01 February 2020, UT1-UTC=-0.1911703, Accumulated Leap Seconds=37
    { 2458881.50, 69.375720 }, //02 February 2020, UT1-UTC=-0.1917202, Accumulated Leap Seconds=37
    { 2458882.50, 69.376187 }, //03 February 2020, UT1-UTC=-0.1921865, Accumulated Leap Seconds=37
    { 2458883.50, 69.376514 }, //04 February 2020, UT1-UTC=-0.1925140, Accumulated Leap Seconds=37
    { 2458884.50, 69.376715 }, //05 February 2020, UT1-UTC=-0.1927151, Accumulated Leap Seconds=37
    { 2458885.50, 69.376845 }, //06 February 2020, UT1-UTC=-0.1928451, Accumulated Leap Seconds=37
    { 2458886.50, 69.376930 }, //07 February 2020, UT1-UTC=-0.1929304, Accumulated Leap Seconds=37
    { 2458887.50, 69.377019 }, //08 February 2020, UT1-UTC=-0.1930187, Accumulated Leap Seconds=37
    { 2458888.50, 69.377268 }, //09 February 2020, UT1-UTC=-0.1932681, Accumulated Leap Seconds=37
    { 2458889.50, 69.377793 }, //10 February 2020, UT1-UTC=-0.1937929, Accumulated Leap Seconds=37
    { 2458890.50, 69.378581 }, //11 February 2020, UT1-UTC=-0.1945811, Accumulated Leap Seconds=37
    { 2458891.50, 69.379552 }, //12 February 2020, UT1-UTC=-0.1955524, Accumulated Leap Seconds=37
    { 2458892.50, 69.380540 }, //13 February 2020, UT1-UTC=-0.1965396, Accumulated Leap Seconds=37
    { 2458893.50, 69.381428 }, //14 February 2020, UT1-UTC=-0.1974284, Accumulated Leap Seconds=37
    { 2458894.50, 69.382113 }, //15 February 2020, UT1-UTC=-0.1981127, Accumulated Leap Seconds=37
    { 2458895.50, 69.382629 }, //16 February 2020, UT1-UTC=-0.1986289, Accumulated Leap Seconds=37
    { 2458896.50, 69.382940 }, //17 February 2020, UT1-UTC=-0.1989401, Accumulated Leap Seconds=37
    { 2458897.50, 69.383076 }, //18 February 2020, UT1-UTC=-0.1990761, Accumulated Leap Seconds=37
    { 2458898.50, 69.383126 }, //19 February 2020, UT1-UTC=-0.1991257, Accumulated Leap Seconds=37
    { 2458899.50, 69.383152 }, //20 February 2020, UT1-UTC=-0.1991524, Accumulated Leap Seconds=37
    { 2458900.50, 69.383237 }, //21 February 2020, UT1-UTC=-0.1992371, Accumulated Leap Seconds=37
    { 2458901.50, 69.383508 }, //22 February 2020, UT1-UTC=-0.1995078, Accumulated Leap Seconds=37
    { 2458902.50, 69.383974 }, //23 February 2020, UT1-UTC=-0.1999742, Accumulated Leap Seconds=37
    { 2458903.50, 69.384575 }, //24 February 2020, UT1-UTC=-0.2005754, Accumulated Leap Seconds=37
    { 2458904.50, 69.385249 }, //25 February 2020, UT1-UTC=-0.2012489, Accumulated Leap Seconds=37
    { 2458905.50, 69.385971 }, //26 February 2020, UT1-UTC=-0.2019712, Accumulated Leap Seconds=37
    { 2458906.50, 69.386736 }, //27 February 2020, UT1-UTC=-0.2027359, Accumulated Leap Seconds=37
    { 2458907.50, 69.387521 }, //28 February 2020, UT1-UTC=-0.2035207, Accumulated Leap Seconds=37
    { 2458908.50, 69.388280 }, //29 February 2020, UT1-UTC=-0.2042796, Accumulated Leap Seconds=37
    { 2458909.50, 69.388990 }, //01 March 2020, UT1-UTC=-0.2049904, Accumulated Leap Seconds=37
    { 2458910.50, 69.389625 }, //02 March 2020, UT1-UTC=-0.2056255, Accumulated Leap Seconds=37
    { 2458911.50, 69.390169 }, //03 March 2020, UT1-UTC=-0.2061689, Accumulated Leap Seconds=37
    { 2458912.50, 69.390666 }, //04 March 2020, UT1-UTC=-0.2066659, Accumulated Leap Seconds=37
    { 2458913.50, 69.391141 }, //05 March 2020, UT1-UTC=-0.2071407, Accumulated Leap Seconds=37
    { 2458914.50, 69.391683 }, //06 March 2020, UT1-UTC=-0.2076825, Accumulated Leap Seconds=37
    { 2458915.50, 69.392381 }, //07 March 2020, UT1-UTC=-0.2083814, Accumulated Leap Seconds=37
    { 2458916.50, 69.393258 }, //08 March 2020, UT1-UTC=-0.2092576, Accumulated Leap Seconds=37
    { 2458917.50, 69.394358 }, //09 March 2020, UT1-UTC=-0.2103578, Accumulated Leap Seconds=37
    { 2458918.50, 69.395715 }, //10 March 2020, UT1-UTC=-0.2117147, Accumulated Leap Seconds=37
    { 2458919.50, 69.397267 }, //11 March 2020, UT1-UTC=-0.2132675, Accumulated Leap Seconds=37
    { 2458920.50, 69.398795 }, //12 March 2020, UT1-UTC=-0.2147950, Accumulated Leap Seconds=37
    { 2458921.50, 69.400140 }, //13 March 2020, UT1-UTC=-0.2161404, Accumulated Leap Seconds=37
    { 2458922.50, 69.401282 }, //14 March 2020, UT1-UTC=-0.2172820, Accumulated Leap Seconds=37
    { 2458923.50, 69.402166 }, //15 March 2020, UT1-UTC=-0.2181664, Accumulated Leap Seconds=37
    { 2458924.50, 69.402794 }, //16 March 2020, UT1-UTC=-0.2187942, Accumulated Leap Seconds=37
    { 2458925.50, 69.403272 }, //17 March 2020, UT1-UTC=-0.2192723, Accumulated Leap Seconds=37
    { 2458926.50, 69.403647 }, //18 March 2020, UT1-UTC=-0.2196470, Accumulated Leap Seconds=37
    { 2458927.50, 69.403942 }, //19 March 2020, UT1-UTC=-0.2199424, Accumulated Leap Seconds=37
    { 2458928.50, 69.404242 }, //20 March 2020, UT1-UTC=-0.2202417, Accumulated Leap Seconds=37
    { 2458929.50, 69.404585 }, //21 March 2020, UT1-UTC=-0.2205851, Accumulated Leap Seconds=37
    { 2458930.50, 69.404949 }, //22 March 2020, UT1-UTC=-0.2209490, Accumulated Leap Seconds=37
    { 2458931.50, 69.405378 }, //23 March 2020, UT1-UTC=-0.2213784, Accumulated Leap Seconds=37
    { 2458932.50, 69.405931 }, //24 March 2020, UT1-UTC=-0.2219310, Accumulated Leap Seconds=37
    { 2458933.50, 69.406494 }, //25 March 2020, UT1-UTC=-0.2224940, Accumulated Leap Seconds=37
    { 2458934.50, 69.407053 }, //26 March 2020, UT1-UTC=-0.2230530, Accumulated Leap Seconds=37
    { 2458935.50, 69.407573 }, //27 March 2020, UT1-UTC=-0.2235729, Accumulated Leap Seconds=37
    { 2458936.50, 69.408033 }, //28 March 2020, UT1-UTC=-0.2240333, Accumulated Leap Seconds=37
    { 2458937.50, 69.408433 }, //29 March 2020, UT1-UTC=-0.2244327, Accumulated Leap Seconds=37
    { 2458938.50, 69.408710 }, //30 March 2020, UT1-UTC=-0.2247096, Accumulated Leap Seconds=37
    { 2458939.50, 69.408943 }, //31 March 2020, UT1-UTC=-0.2249429, Accumulated Leap Seconds=37
    { 2458940.50, 69.409164 }, //01 April 2020, UT1-UTC=-0.2251639, Accumulated Leap Seconds=37
    { 2458941.50, 69.409438 }, //02 April 2020, UT1-UTC=-0.2254376, Accumulated Leap Seconds=37
    { 2458942.50, 69.409815 }, //03 April 2020, UT1-UTC=-0.2258149, Accumulated Leap Seconds=37
    { 2458943.50, 69.410426 }, //04 April 2020, UT1-UTC=-0.2264256, Accumulated Leap Seconds=37
    { 2458944.50, 69.411327 }, //05 April 2020, UT1-UTC=-0.2273265, Accumulated Leap Seconds=37
    { 2458945.50, 69.412545 }, //06 April 2020, UT1-UTC=-0.2285447, Accumulated Leap Seconds=37
    { 2458946.50, 69.414009 }, //07 April 2020, UT1-UTC=-0.2300093, Accumulated Leap Seconds=37
    { 2458947.50, 69.415621 }, //08 April 2020, UT1-UTC=-0.2316212, Accumulated Leap Seconds=37
    { 2458948.50, 69.417204 }, //09 April 2020, UT1-UTC=-0.2332043, Accumulated Leap Seconds=37
    { 2458949.50, 69.418632 }, //10 April 2020, UT1-UTC=-0.2346315, Accumulated Leap Seconds=37
    { 2458950.50, 69.419776 }, //11 April 2020, UT1-UTC=-0.2357756, Accumulated Leap Seconds=37
    { 2458951.50, 69.420627 }, //12 April 2020, UT1-UTC=-0.2366269, Accumulated Leap Seconds=37
    { 2458952.50, 69.421231 }, //13 April 2020, UT1-UTC=-0.2372315, Accumulated Leap Seconds=37
    { 2458953.50, 69.421621 }, //14 April 2020, UT1-UTC=-0.2376207, Accumulated Leap Seconds=37
    { 2458954.50, 69.421856 }, //15 April 2020, UT1-UTC=-0.2378557, Accumulated Leap Seconds=37
    { 2458955.50, 69.422017 }, //16 April 2020, UT1-UTC=-0.2380168, Accumulated Leap Seconds=37
    { 2458956.50, 69.422233 }, //17 April 2020, UT1-UTC=-0.2382326, Accumulated Leap Seconds=37
    { 2458957.50, 69.422573 }, //18 April 2020, UT1-UTC=-0.2385729, Accumulated Leap Seconds=37
    { 2458958.50, 69.423014 }, //19 April 2020, UT1-UTC=-0.2390137, Accumulated Leap Seconds=37
    { 2458959.50, 69.423515 }, //20 April 2020, UT1-UTC=-0.2395153, Accumulated Leap Seconds=37
    { 2458960.50, 69.423995 }, //21 April 2020, UT1-UTC=-0.2399946, Accumulated Leap Seconds=37
    { 2458961.50, 69.424481 }, //22 April 2020, UT1-UTC=-0.2404811, Accumulated Leap Seconds=37
    { 2458962.50, 69.424961 }, //23 April 2020, UT1-UTC=-0.2409606, Accumulated Leap Seconds=37
    { 2458963.50, 69.425370 }, //24 April 2020, UT1-UTC=-0.2413705, Accumulated Leap Seconds=37
    { 2458964.50, 69.425680 }, //25 April 2020, UT1-UTC=-0.2416796, Accumulated Leap Seconds=37
    { 2458965.50, 69.425868 }, //26 April 2020, UT1-UTC=-0.2418681, Accumulated Leap Seconds=37
    { 2458966.50, 69.425977 }, //27 April 2020, UT1-UTC=-0.2419768, Accumulated Leap Seconds=37
    { 2458967.50, 69.426069 }, //28 April 2020, UT1-UTC=-0.2420687, Accumulated Leap Seconds=37
    { 2458968.50, 69.426135 }, //29 April 2020, UT1-UTC=-0.2421354, Accumulated Leap Seconds=37
    { 2458969.50, 69.426237 }, //30 April 2020, UT1-UTC=-0.2422373, Accumulated Leap Seconds=37
    { 2458970.50, 69.426466 }, //01 May 2020, UT1-UTC=-0.2424662, Accumulated Leap Seconds=37
    { 2458971.50, 69.426919 }, //02 May 2020, UT1-UTC=-0.2429192, Accumulated Leap Seconds=37
    { 2458972.50, 69.427644 }, //03 May 2020, UT1-UTC=-0.2436439, Accumulated Leap Seconds=37
    { 2458973.50, 69.428598 }, //04 May 2020, UT1-UTC=-0.2445982, Accumulated Leap Seconds=37
    { 2458974.50, 69.429674 }, //05 May 2020, UT1-UTC=-0.2456736, Accumulated Leap Seconds=37
    { 2458975.50, 69.430782 }, //06 May 2020, UT1-UTC=-0.2467817, Accumulated Leap Seconds=37
    { 2458976.50, 69.431845 }, //07 May 2020, UT1-UTC=-0.2478452, Accumulated Leap Seconds=37
    { 2458977.50, 69.432699 }, //08 May 2020, UT1-UTC=-0.2486991, Accumulated Leap Seconds=37
    { 2458978.50, 69.433290 }, //09 May 2020, UT1-UTC=-0.2492900, Accumulated Leap Seconds=37
    { 2458979.50, 69.433739 }, //10 May 2020, UT1-UTC=-0.2497392, Accumulated Leap Seconds=37
    { 2458980.50, 69.433995 }, //11 May 2020, UT1-UTC=-0.2499950, Accumulated Leap Seconds=37
    { 2458981.50, 69.434153 }, //12 May 2020, UT1-UTC=-0.2501527, Accumulated Leap Seconds=37
    { 2458982.50, 69.434330 }, //13 May 2020, UT1-UTC=-0.2503303, Accumulated Leap Seconds=37
    { 2458983.50, 69.434535 }, //14 May 2020, UT1-UTC=-0.2505353, Accumulated Leap Seconds=37
    { 2458984.50, 69.434752 }, //15 May 2020, UT1-UTC=-0.2507515, Accumulated Leap Seconds=37
    { 2458985.50, 69.435076 }, //16 May 2020, UT1-UTC=-0.2510759, Accumulated Leap Seconds=37
    { 2458986.50, 69.435486 }, //17 May 2020, UT1-UTC=-0.2514862, Accumulated Leap Seconds=37
    { 2458987.50, 69.436004 }, //18 May 2020, UT1-UTC=-0.2520039, Accumulated Leap Seconds=37
    { 2458988.50, 69.436609 }, //19 May 2020, UT1-UTC=-0.2526091, Accumulated Leap Seconds=37
    { 2458989.50, 69.437142 }, //20 May 2020, UT1-UTC=-0.2531420, Accumulated Leap Seconds=37
    { 2458990.50, 69.437620 }, //21 May 2020, UT1-UTC=-0.2536200, Accumulated Leap Seconds=37
    { 2458991.50, 69.437967 }, //22 May 2020, UT1-UTC=-0.2539670, Accumulated Leap Seconds=37
    { 2458992.50, 69.438144 }, //23 May 2020, UT1-UTC=-0.2541436, Accumulated Leap Seconds=37
    { 2458993.50, 69.438153 }, //24 May 2020, UT1-UTC=-0.2541533, Accumulated Leap Seconds=37
    { 2458994.50, 69.438012 }, //25 May 2020, UT1-UTC=-0.2540118, Accumulated Leap Seconds=37
    { 2458995.50, 69.437832 }, //26 May 2020, UT1-UTC=-0.2538321, Accumulated Leap Seconds=37
    { 2458996.50, 69.437679 }, //27 May 2020, UT1-UTC=-0.2536790, Accumulated Leap Seconds=37
    { 2458997.50, 69.437550 }, //28 May 2020, UT1-UTC=-0.2535495, Accumulated Leap Seconds=37
    { 2458998.50, 69.437550 }, //29 May 2020, UT1-UTC=-0.2535504, Accumulated Leap Seconds=37
    { 2458999.50, 69.437714 }, //30 May 2020, UT1-UTC=-0.2537140, Accumulated Leap Seconds=37
    { 2459000.50, 69.438087 }, //31 May 2020, UT1-UTC=-0.2540873, Accumulated Leap Seconds=37
    { 2459001.50, 69.438633 }, //01 June 2020, UT1-UTC=-0.2546335, Accumulated Leap Seconds=37
    { 2459002.50, 69.439252 }, //02 June 2020, UT1-UTC=-0.2552518, Accumulated Leap Seconds=37
    { 2459003.50, 69.439800 }, //03 June 2020, UT1-UTC=-0.2558002, Accumulated Leap Seconds=37
    { 2459004.50, 69.440136 }, //04 June 2020, UT1-UTC=-0.2561358, Accumulated Leap Seconds=37
    { 2459005.50, 69.440208 }, //05 June 2020, UT1-UTC=-0.2562081, Accumulated Leap Seconds=37
    { 2459006.50, 69.440053 }, //06 June 2020, UT1-UTC=-0.2560530, Accumulated Leap Seconds=37
    { 2459007.50, 69.439689 }, //07 June 2020, UT1-UTC=-0.2556889, Accumulated Leap Seconds=37
    { 2459008.50, 69.439185 }, //08 June 2020, UT1-UTC=-0.2551855, Accumulated Leap Seconds=37
    { 2459009.50, 69.438626 }, //09 June 2020, UT1-UTC=-0.2546259, Accumulated Leap Seconds=37
    { 2459010.50, 69.437981 }, //10 June 2020, UT1-UTC=-0.2539811, Accumulated Leap Seconds=37
    { 2459011.50, 69.437376 }, //11 June 2020, UT1-UTC=-0.2533761, Accumulated Leap Seconds=37
    { 2459012.50, 69.436731 }, //12 June 2020, UT1-UTC=-0.2527310, Accumulated Leap Seconds=37
    { 2459013.50, 69.436098 }, //13 June 2020, UT1-UTC=-0.2520982, Accumulated Leap Seconds=37
    { 2459014.50, 69.435564 }, //14 June 2020, UT1-UTC=-0.2515642, Accumulated Leap Seconds=37
    { 2459015.50, 69.435133 }, //15 June 2020, UT1-UTC=-0.2511335, Accumulated Leap Seconds=37
    { 2459016.50, 69.434749 }, //16 June 2020, UT1-UTC=-0.2507493, Accumulated Leap Seconds=37
    { 2459017.50, 69.434280 }, //17 June 2020, UT1-UTC=-0.2502801, Accumulated Leap Seconds=37
    { 2459018.50, 69.433709 }, //18 June 2020, UT1-UTC=-0.2497090, Accumulated Leap Seconds=37
    { 2459019.50, 69.433032 }, //19 June 2020, UT1-UTC=-0.2490317, Accumulated Leap Seconds=37
    { 2459020.50, 69.432102 }, //20 June 2020, UT1-UTC=-0.2481022, Accumulated Leap Seconds=37
    { 2459021.50, 69.431045 }, //21 June 2020, UT1-UTC=-0.2470448, Accumulated Leap Seconds=37
    { 2459022.50, 69.429886 }, //22 June 2020, UT1-UTC=-0.2458865, Accumulated Leap Seconds=37
    { 2459023.50, 69.428702 }, //23 June 2020, UT1-UTC=-0.2447016, Accumulated Leap Seconds=37
    { 2459024.50, 69.427573 }, //24 June 2020, UT1-UTC=-0.2435726, Accumulated Leap Seconds=37
    { 2459025.50, 69.426600 }, //25 June 2020, UT1-UTC=-0.2426000, Accumulated Leap Seconds=37
    { 2459026.50, 69.425866 }, //26 June 2020, UT1-UTC=-0.2418664, Accumulated Leap Seconds=37
    { 2459027.50, 69.425362 }, //27 June 2020, UT1-UTC=-0.2413618, Accumulated Leap Seconds=37
    { 2459028.50, 69.425025 }, //28 June 2020, UT1-UTC=-0.2410246, Accumulated Leap Seconds=37
    { 2459029.50, 69.424810 }, //29 June 2020, UT1-UTC=-0.2408104, Accumulated Leap Seconds=37
    { 2459030.50, 69.424568 }, //30 June 2020, UT1-UTC=-0.2405680, Accumulated Leap Seconds=37
    { 2459031.50, 69.424133 }, //01 July 2020, UT1-UTC=-0.2401335, Accumulated Leap Seconds=37
    { 2459032.50, 69.423485 }, //02 July 2020, UT1-UTC=-0.2394847, Accumulated Leap Seconds=37
    { 2459033.50, 69.422600 }, //03 July 2020, UT1-UTC=-0.2385999, Accumulated Leap Seconds=37
    { 2459034.50, 69.421499 }, //04 July 2020, UT1-UTC=-0.2374993, Accumulated Leap Seconds=37
    { 2459035.50, 69.420218 }, //05 July 2020, UT1-UTC=-0.2362178, Accumulated Leap Seconds=37
    { 2459036.50, 69.418878 }, //06 July 2020, UT1-UTC=-0.2348785, Accumulated Leap Seconds=37
    { 2459037.50, 69.417601 }, //07 July 2020, UT1-UTC=-0.2336011, Accumulated Leap Seconds=37
    { 2459038.50, 69.416395 }, //08 July 2020, UT1-UTC=-0.2323948, Accumulated Leap Seconds=37
    { 2459039.50, 69.415266 }, //09 July 2020, UT1-UTC=-0.2312661, Accumulated Leap Seconds=37
    { 2459040.50, 69.414208 }, //10 July 2020, UT1-UTC=-0.2302077, Accumulated Leap Seconds=37
    { 2459041.50, 69.413242 }, //11 July 2020, UT1-UTC=-0.2292418, Accumulated Leap Seconds=37
    { 2459042.50, 69.412237 }, //12 July 2020, UT1-UTC=-0.2282373, Accumulated Leap Seconds=37
    { 2459043.50, 69.411192 }, //13 July 2020, UT1-UTC=-0.2271922, Accumulated Leap Seconds=37
    { 2459044.50, 69.410120 }, //14 July 2020, UT1-UTC=-0.2261196, Accumulated Leap Seconds=37
    { 2459045.50, 69.408965 }, //15 July 2020, UT1-UTC=-0.2249647, Accumulated Leap Seconds=37
    { 2459046.50, 69.407756 }, //16 July 2020, UT1-UTC=-0.2237563, Accumulated Leap Seconds=37
    { 2459047.50, 69.406474 }, //17 July 2020, UT1-UTC=-0.2224735, Accumulated Leap Seconds=37
    { 2459048.50, 69.405122 }, //18 July 2020, UT1-UTC=-0.2211221, Accumulated Leap Seconds=37
    { 2459049.50, 69.403697 }, //19 July 2020, UT1-UTC=-0.2196966, Accumulated Leap Seconds=37
    { 2459050.50, 69.402223 }, //20 July 2020, UT1-UTC=-0.2182229, Accumulated Leap Seconds=37
    { 2459051.50, 69.400857 }, //21 July 2020, UT1-UTC=-0.2168569, Accumulated Leap Seconds=37
    { 2459052.50, 69.399693 }, //22 July 2020, UT1-UTC=-0.2156934, Accumulated Leap Seconds=37
    { 2459053.50, 69.398740 }, //23 July 2020, UT1-UTC=-0.2147397, Accumulated Leap Seconds=37
    { 2459054.50, 69.397989 }, //24 July 2020, UT1-UTC=-0.2139895, Accumulated Leap Seconds=37
    { 2459055.50, 69.397428 }, //25 July 2020, UT1-UTC=-0.2134277, Accumulated Leap Seconds=37
    { 2459056.50, 69.396964 }, //26 July 2020, UT1-UTC=-0.2129642, Accumulated Leap Seconds=37
    { 2459057.50, 69.396451 }, //27 July 2020, UT1-UTC=-0.2124511, Accumulated Leap Seconds=37
    { 2459058.50, 69.395825 }, //28 July 2020, UT1-UTC=-0.2118250, Accumulated Leap Seconds=37
    { 2459059.50, 69.395086 }, //29 July 2020, UT1-UTC=-0.2110863, Accumulated Leap Seconds=37
    { 2459060.50, 69.394185 }, //30 July 2020, UT1-UTC=-0.2101852, Accumulated Leap Seconds=37
    { 2459061.50, 69.393173 }, //31 July 2020, UT1-UTC=-0.2091731, Accumulated Leap Seconds=37
    { 2459062.50, 69.392124 }, //01 August 2020, UT1-UTC=-0.2081241, Accumulated Leap Seconds=37
    { 2459063.50, 69.391052 }, //02 August 2020, UT1-UTC=-0.2070517, Accumulated Leap Seconds=37
    { 2459064.50, 69.390068 }, //03 August 2020, UT1-UTC=-0.2060679, Accumulated Leap Seconds=37
    { 2459065.50, 69.389219 }, //04 August 2020, UT1-UTC=-0.2052187, Accumulated Leap Seconds=37
    { 2459066.50, 69.388463 }, //05 August 2020, UT1-UTC=-0.2044629, Accumulated Leap Seconds=37
    { 2459067.50, 69.387881 }, //06 August 2020, UT1-UTC=-0.2038807, Accumulated Leap Seconds=37
    { 2459068.50, 69.387420 }, //07 August 2020, UT1-UTC=-0.2034204, Accumulated Leap Seconds=37
    { 2459069.50, 69.387041 }, //08 August 2020, UT1-UTC=-0.2030407, Accumulated Leap Seconds=37
    { 2459070.50, 69.386727 }, //09 August 2020, UT1-UTC=-0.2027274, Accumulated Leap Seconds=37
    { 2459071.50, 69.386337 }, //10 August 2020, UT1-UTC=-0.2023368, Accumulated Leap Seconds=37
    { 2459072.50, 69.385832 }, //11 August 2020, UT1-UTC=-0.2018318, Accumulated Leap Seconds=37
    { 2459073.50, 69.385197 }, //12 August 2020, UT1-UTC=-0.2011971, Accumulated Leap Seconds=37
    { 2459074.50, 69.384391 }, //13 August 2020, UT1-UTC=-0.2003908, Accumulated Leap Seconds=37
    { 2459075.50, 69.383493 }, //14 August 2020, UT1-UTC=-0.1994927, Accumulated Leap Seconds=37
    { 2459076.50, 69.382461 }, //15 August 2020, UT1-UTC=-0.1984615, Accumulated Leap Seconds=37
    { 2459077.50, 69.381279 }, //16 August 2020, UT1-UTC=-0.1972792, Accumulated Leap Seconds=37
    { 2459078.50, 69.380107 }, //17 August 2020, UT1-UTC=-0.1961068, Accumulated Leap Seconds=37
    { 2459079.50, 69.379096 }, //18 August 2020, UT1-UTC=-0.1950958, Accumulated Leap Seconds=37
    { 2459080.50, 69.378295 }, //19 August 2020, UT1-UTC=-0.1942946, Accumulated Leap Seconds=37
    { 2459081.50, 69.377665 }, //20 August 2020, UT1-UTC=-0.1936647, Accumulated Leap Seconds=37
    { 2459082.50, 69.377349 }, //21 August 2020, UT1-UTC=-0.1933494, Accumulated Leap Seconds=37
    { 2459083.50, 69.377238 }, //22 August 2020, UT1-UTC=-0.1932377, Accumulated Leap Seconds=37
    { 2459084.50, 69.377178 }, //23 August 2020, UT1-UTC=-0.1931778, Accumulated Leap Seconds=37
    { 2459085.50, 69.377031 }, //24 August 2020, UT1-UTC=-0.1930310, Accumulated Leap Seconds=37
    { 2459086.50, 69.376704 }, //25 August 2020, UT1-UTC=-0.1927039, Accumulated Leap Seconds=37
    { 2459087.50, 69.376095 }, //26 August 2020, UT1-UTC=-0.1920949, Accumulated Leap Seconds=37
    { 2459088.50, 69.375202 }, //27 August 2020, UT1-UTC=-0.1912021, Accumulated Leap Seconds=37
    { 2459089.50, 69.374122 }, //28 August 2020, UT1-UTC=-0.1901225, Accumulated Leap Seconds=37
    { 2459090.50, 69.372891 }, //29 August 2020, UT1-UTC=-0.1888915, Accumulated Leap Seconds=37
    { 2459091.50, 69.371601 }, //30 August 2020, UT1-UTC=-0.1876013, Accumulated Leap Seconds=37
    { 2459092.50, 69.370382 }, //31 August 2020, UT1-UTC=-0.1863822, Accumulated Leap Seconds=37
    { 2459093.50, 69.369342 }, //01 September 2020, UT1-UTC=-0.1853422, Accumulated Leap Seconds=37
    { 2459094.50, 69.368421 }, //02 September 2020, UT1-UTC=-0.1844207, Accumulated Leap Seconds=37
    { 2459095.50, 69.367644 }, //03 September 2020, UT1-UTC=-0.1836441, Accumulated Leap Seconds=37
    { 2459096.50, 69.367008 }, //04 September 2020, UT1-UTC=-0.1830079, Accumulated Leap Seconds=37
    { 2459097.50, 69.366438 }, //05 September 2020, UT1-UTC=-0.1824377, Accumulated Leap Seconds=37
    { 2459098.50, 69.365864 }, //06 September 2020, UT1-UTC=-0.1818637, Accumulated Leap Seconds=37
    { 2459099.50, 69.365304 }, //07 September 2020, UT1-UTC=-0.1813038, Accumulated Leap Seconds=37
    { 2459100.50, 69.364675 }, //08 September 2020, UT1-UTC=-0.1806747, Accumulated Leap Seconds=37
    { 2459101.50, 69.363960 }, //09 September 2020, UT1-UTC=-0.1799600, Accumulated Leap Seconds=37
    { 2459102.50, 69.363144 }, //10 September 2020, UT1-UTC=-0.1791442, Accumulated Leap Seconds=37
    { 2459103.50, 69.362349 }, //11 September 2020, UT1-UTC=-0.1783491, Accumulated Leap Seconds=37
    { 2459104.50, 69.361533 }, //12 September 2020, UT1-UTC=-0.1775331, Accumulated Leap Seconds=37
    { 2459105.50, 69.360742 }, //13 September 2020, UT1-UTC=-0.1767424, Accumulated Leap Seconds=37
    { 2459106.50, 69.360111 }, //14 September 2020, UT1-UTC=-0.1761111, Accumulated Leap Seconds=37
    { 2459107.50, 69.359711 }, //15 September 2020, UT1-UTC=-0.1757107, Accumulated Leap Seconds=37
    { 2459108.50, 69.359555 }, //16 September 2020, UT1-UTC=-0.1755555, Accumulated Leap Seconds=37
    { 2459109.50, 69.359676 }, //17 September 2020, UT1-UTC=-0.1756758, Accumulated Leap Seconds=37
    { 2459110.50, 69.359978 }, //18 September 2020, UT1-UTC=-0.1759784, Accumulated Leap Seconds=37
    { 2459111.50, 69.360403 }, //19 September 2020, UT1-UTC=-0.1764030, Accumulated Leap Seconds=37
    { 2459112.50, 69.360789 }, //20 September 2020, UT1-UTC=-0.1767895, Accumulated Leap Seconds=37
    { 2459113.50, 69.361006 }, //21 September 2020, UT1-UTC=-0.1770063, Accumulated Leap Seconds=37
    { 2459114.50, 69.361019 }, //22 September 2020, UT1-UTC=-0.1770188, Accumulated Leap Seconds=37
    { 2459115.50, 69.360811 }, //23 September 2020, UT1-UTC=-0.1768106, Accumulated Leap Seconds=37
    { 2459116.50, 69.360457 }, //24 September 2020, UT1-UTC=-0.1764569, Accumulated Leap Seconds=37
    { 2459117.50, 69.360008 }, //25 September 2020, UT1-UTC=-0.1760082, Accumulated Leap Seconds=37
    { 2459118.50, 69.359564 }, //26 September 2020, UT1-UTC=-0.1755640, Accumulated Leap Seconds=37
    { 2459119.50, 69.359037 }, //27 September 2020, UT1-UTC=-0.1750367, Accumulated Leap Seconds=37
    { 2459120.50, 69.358483 }, //28 September 2020, UT1-UTC=-0.1744831, Accumulated Leap Seconds=37
    { 2459121.50, 69.358055 }, //29 September 2020, UT1-UTC=-0.1740546, Accumulated Leap Seconds=37
    { 2459122.50, 69.357718 }, //30 September 2020, UT1-UTC=-0.1737183, Accumulated Leap Seconds=37
    { 2459123.50, 69.357478 }, //01 October 2020, UT1-UTC=-0.1734782, Accumulated Leap Seconds=37
    { 2459124.50, 69.357252 }, //02 October 2020, UT1-UTC=-0.1732518, Accumulated Leap Seconds=37
    { 2459125.50, 69.357089 }, //03 October 2020, UT1-UTC=-0.1730890, Accumulated Leap Seconds=37
    { 2459126.50, 69.356952 }, //04 October 2020, UT1-UTC=-0.1729516, Accumulated Leap Seconds=37
    { 2459127.50, 69.356781 }, //05 October 2020, UT1-UTC=-0.1727809, Accumulated Leap Seconds=37
    { 2459128.50, 69.356508 }, //06 October 2020, UT1-UTC=-0.1725084, Accumulated Leap Seconds=37
    { 2459129.50, 69.356119 }, //07 October 2020, UT1-UTC=-0.1721191, Accumulated Leap Seconds=37
    { 2459130.50, 69.355606 }, //08 October 2020, UT1-UTC=-0.1716062, Accumulated Leap Seconds=37
    { 2459131.50, 69.354953 }, //09 October 2020, UT1-UTC=-0.1709530, Accumulated Leap Seconds=37
    { 2459132.50, 69.354345 }, //10 October 2020, UT1-UTC=-0.1703455, Accumulated Leap Seconds=37
    { 2459133.50, 69.353833 }, //11 October 2020, UT1-UTC=-0.1698329, Accumulated Leap Seconds=37
    { 2459134.50, 69.353429 }, //12 October 2020, UT1-UTC=-0.1694286, Accumulated Leap Seconds=37
    { 2459135.50, 69.353290 }, //13 October 2020, UT1-UTC=-0.1692903, Accumulated Leap Seconds=37
    { 2459136.50, 69.353485 }, //14 October 2020, UT1-UTC=-0.1694855, Accumulated Leap Seconds=37
    { 2459137.50, 69.353928 }, //15 October 2020, UT1-UTC=-0.1699282, Accumulated Leap Seconds=37
    { 2459138.50, 69.354637 }, //16 October 2020, UT1-UTC=-0.1706366, Accumulated Leap Seconds=37
    { 2459139.50, 69.355433 }, //17 October 2020, UT1-UTC=-0.1714326, Accumulated Leap Seconds=37
    { 2459140.50, 69.356209 }, //18 October 2020, UT1-UTC=-0.1722094, Accumulated Leap Seconds=37
    { 2459141.50, 69.356737 }, //19 October 2020, UT1-UTC=-0.1727370, Accumulated Leap Seconds=37
    { 2459142.50, 69.357014 }, //20 October 2020, UT1-UTC=-0.1730140, Accumulated Leap Seconds=37
    { 2459143.50, 69.357091 }, //21 October 2020, UT1-UTC=-0.1730915, Accumulated Leap Seconds=37
    { 2459144.50, 69.357076 }, //22 October 2020, UT1-UTC=-0.1730757, Accumulated Leap Seconds=37
    { 2459145.50, 69.357119 }, //23 October 2020, UT1-UTC=-0.1731186, Accumulated Leap Seconds=37
    { 2459146.50, 69.357193 }, //24 October 2020, UT1-UTC=-0.1731930, Accumulated Leap Seconds=37
    { 2459147.50, 69.357321 }, //25 October 2020, UT1-UTC=-0.1733207, Accumulated Leap Seconds=37
    { 2459148.50, 69.357498 }, //26 October 2020, UT1-UTC=-0.1734985, Accumulated Leap Seconds=37
    { 2459149.50, 69.357762 }, //27 October 2020, UT1-UTC=-0.1737620, Accumulated Leap Seconds=37
    { 2459150.50, 69.358107 }, //28 October 2020, UT1-UTC=-0.1741066, Accumulated Leap Seconds=37
    { 2459151.50, 69.358464 }, //29 October 2020, UT1-UTC=-0.1744639, Accumulated Leap Seconds=37
    { 2459152.50, 69.358771 }, //30 October 2020, UT1-UTC=-0.1747709, Accumulated Leap Seconds=37
    { 2459153.50, 69.359062 }, //31 October 2020, UT1-UTC=-0.1750625, Accumulated Leap Seconds=37
    { 2459154.50, 69.359324 }, //01 November 2020, UT1-UTC=-0.1753242, Accumulated Leap Seconds=37
    { 2459155.50, 69.359492 }, //02 November 2020, UT1-UTC=-0.1754919, Accumulated Leap Seconds=37
    { 2459156.50, 69.359538 }, //03 November 2020, UT1-UTC=-0.1755382, Accumulated Leap Seconds=37
    { 2459157.50, 69.359484 }, //04 November 2020, UT1-UTC=-0.1754841, Accumulated Leap Seconds=37
    { 2459158.50, 69.359309 }, //05 November 2020, UT1-UTC=-0.1753093, Accumulated Leap Seconds=37
    { 2459159.50, 69.359030 }, //06 November 2020, UT1-UTC=-0.1750298, Accumulated Leap Seconds=37
    { 2459160.50, 69.358723 }, //07 November 2020, UT1-UTC=-0.1747231, Accumulated Leap Seconds=37
    { 2459161.50, 69.358517 }, //08 November 2020, UT1-UTC=-0.1745166, Accumulated Leap Seconds=37
    { 2459162.50, 69.358566 }, //09 November 2020, UT1-UTC=-0.1745658, Accumulated Leap Seconds=37
    { 2459163.50, 69.358925 }, //10 November 2020, UT1-UTC=-0.1749252, Accumulated Leap Seconds=37
    { 2459164.50, 69.359491 }, //11 November 2020, UT1-UTC=-0.1754913, Accumulated Leap Seconds=37
    { 2459165.50, 69.360261 }, //12 November 2020, UT1-UTC=-0.1762615, Accumulated Leap Seconds=37
    { 2459166.50, 69.361147 }, //13 November 2020, UT1-UTC=-0.1771474, Accumulated Leap Seconds=37
    { 2459167.50, 69.361973 }, //14 November 2020, UT1-UTC=-0.1779726, Accumulated Leap Seconds=37
    { 2459168.50, 69.362636 }, //15 November 2020, UT1-UTC=-0.1786358, Accumulated Leap Seconds=37
    { 2459169.50, 69.363014 }, //16 November 2020, UT1-UTC=-0.1790143, Accumulated Leap Seconds=37
    { 2459170.50, 69.363119 }, //17 November 2020, UT1-UTC=-0.1791188, Accumulated Leap Seconds=37
    { 2459171.50, 69.363020 }, //18 November 2020, UT1-UTC=-0.1790201, Accumulated Leap Seconds=37
    { 2459172.50, 69.362745 }, //19 November 2020, UT1-UTC=-0.1787447, Accumulated Leap Seconds=37
    { 2459173.50, 69.362450 }, //20 November 2020, UT1-UTC=-0.1784505, Accumulated Leap Seconds=37
    { 2459174.50, 69.362194 }, //21 November 2020, UT1-UTC=-0.1781942, Accumulated Leap Seconds=37
    { 2459175.50, 69.362074 }, //22 November 2020, UT1-UTC=-0.1780743, Accumulated Leap Seconds=37
    { 2459176.50, 69.362147 }, //23 November 2020, UT1-UTC=-0.1781469, Accumulated Leap Seconds=37
    { 2459177.50, 69.362355 }, //24 November 2020, UT1-UTC=-0.1783547, Accumulated Leap Seconds=37
    { 2459178.50, 69.362641 }, //25 November 2020, UT1-UTC=-0.1786407, Accumulated Leap Seconds=37
    { 2459179.50, 69.362963 }, //26 November 2020, UT1-UTC=-0.1789629, Accumulated Leap Seconds=37
    { 2459180.50, 69.363135 }, //27 November 2020, UT1-UTC=-0.1791349, Accumulated Leap Seconds=37
    { 2459181.50, 69.363286 }, //28 November 2020, UT1-UTC=-0.1792864, Accumulated Leap Seconds=37
    { 2459182.50, 69.363305 }, //29 November 2020, UT1-UTC=-0.1793053, Accumulated Leap Seconds=37
    { 2459183.50, 69.363212 }, //30 November 2020, UT1-UTC=-0.1792119, Accumulated Leap Seconds=37
    { 2459184.50, 69.363024 }, //01 December 2020, UT1-UTC=-0.1790244, Accumulated Leap Seconds=37
    { 2459185.50, 69.362717 }, //02 December 2020, UT1-UTC=-0.1787170, Accumulated Leap Seconds=37
    { 2459186.50, 69.362321 }, //03 December 2020, UT1-UTC=-0.1783210, Accumulated Leap Seconds=37
    { 2459187.50, 69.361923 }, //04 December 2020, UT1-UTC=-0.1779234, Accumulated Leap Seconds=37
    { 2459188.50, 69.361628 }, //05 December 2020, UT1-UTC=-0.1776276, Accumulated Leap Seconds=37
    { 2459189.50, 69.361540 }, //06 December 2020, UT1-UTC=-0.1775402, Accumulated Leap Seconds=37
    { 2459190.50, 69.361646 }, //07 December 2020, UT1-UTC=-0.1776460, Accumulated Leap Seconds=37
    { 2459191.50, 69.361974 }, //08 December 2020, UT1-UTC=-0.1779740, Accumulated Leap Seconds=37
    { 2459192.50, 69.362494 }, //09 December 2020, UT1-UTC=-0.1784944, Accumulated Leap Seconds=37
    { 2459193.50, 69.363104 }, //10 December 2020, UT1-UTC=-0.1791038, Accumulated Leap Seconds=37
    { 2459194.50, 69.363735 }, //11 December 2020, UT1-UTC=-0.1797353, Accumulated Leap Seconds=37
    { 2459195.50, 69.364180 }, //12 December 2020, UT1-UTC=-0.1801801, Accumulated Leap Seconds=37
    { 2459196.50, 69.364399 }, //13 December 2020, UT1-UTC=-0.1803991, Accumulated Leap Seconds=37
    { 2459197.50, 69.364377 }, //14 December 2020, UT1-UTC=-0.1803768, Accumulated Leap Seconds=37
    { 2459198.50, 69.364117 }, //15 December 2020, UT1-UTC=-0.1801173, Accumulated Leap Seconds=37
    { 2459199.50, 69.363684 }, //16 December 2020, UT1-UTC=-0.1796837, Accumulated Leap Seconds=37
    { 2459200.50, 69.363165 }, //17 December 2020, UT1-UTC=-0.1791649, Accumulated Leap Seconds=37
    { 2459201.50, 69.362769 }, //18 December 2020, UT1-UTC=-0.1787687, Accumulated Leap Seconds=37
    { 2459202.50, 69.362462 }, //19 December 2020, UT1-UTC=-0.1784625, Accumulated Leap Seconds=37
    { 2459203.50, 69.362312 }, //20 December 2020, UT1-UTC=-0.1783116, Accumulated Leap Seconds=37
    { 2459204.50, 69.362298 }, //21 December 2020, UT1-UTC=-0.1782977, Accumulated Leap Seconds=37
    { 2459205.50, 69.362394 }, //22 December 2020, UT1-UTC=-0.1783938, Accumulated Leap Seconds=37
    { 2459206.50, 69.362563 }, //23 December 2020, UT1-UTC=-0.1785629, Accumulated Leap Seconds=37
    { 2459207.50, 69.362636 }, //24 December 2020, UT1-UTC=-0.1786356, Accumulated Leap Seconds=37
    { 2459208.50, 69.362635 }, //25 December 2020, UT1-UTC=-0.1786349, Accumulated Leap Seconds=37
    { 2459209.50, 69.362503 }, //26 December 2020, UT1-UTC=-0.1785028, Accumulated Leap Seconds=37
    { 2459210.50, 69.362208 }, //27 December 2020, UT1-UTC=-0.1782078, Accumulated Leap Seconds=37
    { 2459211.50, 69.361772 }, //28 December 2020, UT1-UTC=-0.1777715, Accumulated Leap Seconds=37
    { 2459212.50, 69.361239 }, //29 December 2020, UT1-UTC=-0.1772387, Accumulated Leap Seconds=37
    { 2459213.50, 69.360642 }, //30 December 2020, UT1-UTC=-0.1766424, Accumulated Leap Seconds=37
    { 2459214.50, 69.359981 }, //31 December 2020, UT1-UTC=-0.1759809, Accumulated Leap Seconds=37
    { 2459215.50, 69.359334 }, //01 January 2021, UT1-UTC=-0.1753340, Accumulated Leap Seconds=37
    { 2459216.50, 69.358812 }, //02 January 2021, UT1-UTC=-0.1748118, Accumulated Leap Seconds=37
    { 2459217.50, 69.358473 }, //03 January 2021, UT1-UTC=-0.1744728, Accumulated Leap Seconds=37
    { 2459218.50, 69.358326 }, //04 January 2021, UT1-UTC=-0.1743263, Accumulated Leap Seconds=37
    { 2459219.50, 69.358380 }, //05 January 2021, UT1-UTC=-0.1743799, Accumulated Leap Seconds=37
    { 2459220.50, 69.358574 }, //06 January 2021, UT1-UTC=-0.1745738, Accumulated Leap Seconds=37
    { 2459221.50, 69.358835 }, //07 January 2021, UT1-UTC=-0.1748355, Accumulated Leap Seconds=37
    { 2459222.50, 69.359060 }, //08 January 2021, UT1-UTC=-0.1750599, Accumulated Leap Seconds=37
    { 2459223.50, 69.359087 }, //09 January 2021, UT1-UTC=-0.1750871, Accumulated Leap Seconds=37
    { 2459224.50, 69.358867 }, //10 January 2021, UT1-UTC=-0.1748670, Accumulated Leap Seconds=37
    { 2459225.50, 69.358463 }, //11 January 2021, UT1-UTC=-0.1744634, Accumulated Leap Seconds=37
    { 2459226.50, 69.357966 }, //12 January 2021, UT1-UTC=-0.1739663, Accumulated Leap Seconds=37
    { 2459227.50, 69.357417 }, //13 January 2021, UT1-UTC=-0.1734174, Accumulated Leap Seconds=37
    { 2459228.50, 69.356884 }, //14 January 2021, UT1-UTC=-0.1728842, Accumulated Leap Seconds=37
    { 2459229.50, 69.356469 }, //15 January 2021, UT1-UTC=-0.1724694, Accumulated Leap Seconds=37
    { 2459230.50, 69.356040 }, //16 January 2021, UT1-UTC=-0.1720402, Accumulated Leap Seconds=37
    { 2459231.50, 69.355766 }, //17 January 2021, UT1-UTC=-0.1717660, Accumulated Leap Seconds=37
    { 2459232.50, 69.355586 }, //18 January 2021, UT1-UTC=-0.1715857, Accumulated Leap Seconds=37
    { 2459233.50, 69.355416 }, //19 January 2021, UT1-UTC=-0.1714164, Accumulated Leap Seconds=37
    { 2459234.50, 69.355273 }, //20 January 2021, UT1-UTC=-0.1712735, Accumulated Leap Seconds=37
    { 2459235.50, 69.355090 }, //21 January 2021, UT1-UTC=-0.1710897, Accumulated Leap Seconds=37
    { 2459236.50, 69.354796 }, //22 January 2021, UT1-UTC=-0.1707958, Accumulated Leap Seconds=37
    { 2459237.50, 69.354479 }, //23 January 2021, UT1-UTC=-0.1704792, Accumulated Leap Seconds=37
    { 2459238.50, 69.354010 }, //24 January 2021, UT1-UTC=-0.1700095, Accumulated Leap Seconds=37
    { 2459239.50, 69.353415 }, //25 January 2021, UT1-UTC=-0.1694151, Accumulated Leap Seconds=37
    { 2459240.50, 69.352720 }, //26 January 2021, UT1-UTC=-0.1687202, Accumulated Leap Seconds=37
    { 2459241.50, 69.351955 }, //27 January 2021, UT1-UTC=-0.1679551, Accumulated Leap Seconds=37
    { 2459242.50, 69.351309 }, //28 January 2021, UT1-UTC=-0.1673088, Accumulated Leap Seconds=37
    { 2459243.50, 69.350911 }, //29 January 2021, UT1-UTC=-0.1669113, Accumulated Leap Seconds=37
    { 2459244.50, 69.350682 }, //30 January 2021, UT1-UTC=-0.1666818, Accumulated Leap Seconds=37
    { 2459245.50, 69.350688 }, //31 January 2021, UT1-UTC=-0.1666882, Accumulated Leap Seconds=37
    { 2459246.50, 69.351013 }, //01 February 2021, UT1-UTC=-0.1670133, Accumulated Leap Seconds=37
    { 2459247.50, 69.351560 }, //02 February 2021, UT1-UTC=-0.1675605, Accumulated Leap Seconds=37
    { 2459248.50, 69.352217 }, //03 February 2021, UT1-UTC=-0.1682174, Accumulated Leap Seconds=37
    { 2459249.50, 69.352836 }, //04 February 2021, UT1-UTC=-0.1688356, Accumulated Leap Seconds=37
    { 2459250.50, 69.353284 }, //05 February 2021, UT1-UTC=-0.1692841, Accumulated Leap Seconds=37
    { 2459251.50, 69.353532 }, //06 February 2021, UT1-UTC=-0.1695316, Accumulated Leap Seconds=37
    { 2459252.50, 69.353577 }, //07 February 2021, UT1-UTC=-0.1695770, Accumulated Leap Seconds=37
    { 2459253.50, 69.353478 }, //08 February 2021, UT1-UTC=-0.1694781, Accumulated Leap Seconds=37
    { 2459254.50, 69.353330 }, //09 February 2021, UT1-UTC=-0.1693298, Accumulated Leap Seconds=37
    { 2459255.50, 69.353252 }, //10 February 2021, UT1-UTC=-0.1692518, Accumulated Leap Seconds=37
    { 2459256.50, 69.353258 }, //11 February 2021, UT1-UTC=-0.1692585, Accumulated Leap Seconds=37
    { 2459257.50, 69.353410 }, //12 February 2021, UT1-UTC=-0.1694104, Accumulated Leap Seconds=37
    { 2459258.50, 69.353718 }, //13 February 2021, UT1-UTC=-0.1697177, Accumulated Leap Seconds=37
    { 2459259.50, 69.354183 }, //14 February 2021, UT1-UTC=-0.1701828, Accumulated Leap Seconds=37
    { 2459260.50, 69.354735 }, //15 February 2021, UT1-UTC=-0.1707346, Accumulated Leap Seconds=37
    { 2459261.50, 69.355227 }, //16 February 2021, UT1-UTC=-0.1712267, Accumulated Leap Seconds=37
    { 2459262.50, 69.355658 }, //17 February 2021, UT1-UTC=-0.1716582, Accumulated Leap Seconds=37
    { 2459263.50, 69.355934 }, //18 February 2021, UT1-UTC=-0.1719340, Accumulated Leap Seconds=37
    { 2459264.50, 69.355996 }, //19 February 2021, UT1-UTC=-0.1719960, Accumulated Leap Seconds=37
    { 2459265.50, 69.355895 }, //20 February 2021, UT1-UTC=-0.1718953, Accumulated Leap Seconds=37
    { 2459266.50, 69.355559 }, //21 February 2021, UT1-UTC=-0.1715594, Accumulated Leap Seconds=37
    { 2459267.50, 69.355042 }, //22 February 2021, UT1-UTC=-0.1710417, Accumulated Leap Seconds=37
    { 2459268.50, 69.354454 }, //23 February 2021, UT1-UTC=-0.1704541, Accumulated Leap Seconds=37
    { 2459269.50, 69.353816 }, //24 February 2021, UT1-UTC=-0.1698157, Accumulated Leap Seconds=37
    { 2459270.50, 69.353332 }, //25 February 2021, UT1-UTC=-0.1693325, Accumulated Leap Seconds=37
    { 2459271.50, 69.353052 }, //26 February 2021, UT1-UTC=-0.1690519, Accumulated Leap Seconds=37
    { 2459272.50, 69.353042 }, //27 February 2021, UT1-UTC=-0.1690420, Accumulated Leap Seconds=37
    { 2459273.50, 69.353289 }, //28 February 2021, UT1-UTC=-0.1692889, Accumulated Leap Seconds=37
    { 2459274.50, 69.353792 }, //01 March 2021, UT1-UTC=-0.1697917, Accumulated Leap Seconds=37
    { 2459275.50, 69.354408 }, //02 March 2021, UT1-UTC=-0.1704083, Accumulated Leap Seconds=37
    { 2459276.50, 69.355007 }, //03 March 2021, UT1-UTC=-0.1710068, Accumulated Leap Seconds=37
    { 2459277.50, 69.355574 }, //04 March 2021, UT1-UTC=-0.1715739, Accumulated Leap Seconds=37
    { 2459278.50, 69.355908 }, //05 March 2021, UT1-UTC=-0.1719077, Accumulated Leap Seconds=37
    { 2459279.50, 69.355959 }, //06 March 2021, UT1-UTC=-0.1719593, Accumulated Leap Seconds=37
    { 2459280.50, 69.355830 }, //07 March 2021, UT1-UTC=-0.1718305, Accumulated Leap Seconds=37
    { 2459281.50, 69.355596 }, //08 March 2021, UT1-UTC=-0.1715956, Accumulated Leap Seconds=37
    { 2459282.50, 69.355314 }, //09 March 2021, UT1-UTC=-0.1713142, Accumulated Leap Seconds=37
    { 2459283.50, 69.355105 }, //10 March 2021, UT1-UTC=-0.1711055, Accumulated Leap Seconds=37
    { 2459284.50, 69.355021 }, //11 March 2021, UT1-UTC=-0.1710208, Accumulated Leap Seconds=37
    { 2459285.50, 69.355074 }, //12 March 2021, UT1-UTC=-0.1710743, Accumulated Leap Seconds=37
    { 2459286.50, 69.355282 }, //13 March 2021, UT1-UTC=-0.1712815, Accumulated Leap Seconds=37
    { 2459287.50, 69.355605 }, //14 March 2021, UT1-UTC=-0.1716048, Accumulated Leap Seconds=37
    { 2459288.50, 69.355944 }, //15 March 2021, UT1-UTC=-0.1719443, Accumulated Leap Seconds=37
    { 2459289.50, 69.356229 }, //16 March 2021, UT1-UTC=-0.1722294, Accumulated Leap Seconds=37
    { 2459290.50, 69.356371 }, //17 March 2021, UT1-UTC=-0.1723712, Accumulated Leap Seconds=37
    { 2459291.50, 69.356377 }, //18 March 2021, UT1-UTC=-0.1723768, Accumulated Leap Seconds=37
    { 2459292.50, 69.356285 }, //19 March 2021, UT1-UTC=-0.1722852, Accumulated Leap Seconds=37
    { 2459293.50, 69.356014 }, //20 March 2021, UT1-UTC=-0.1720139, Accumulated Leap Seconds=37
    { 2459294.50, 69.355668 }, //21 March 2021, UT1-UTC=-0.1716685, Accumulated Leap Seconds=37
    { 2459295.50, 69.355311 }, //22 March 2021, UT1-UTC=-0.1713111, Accumulated Leap Seconds=37
    { 2459296.50, 69.354963 }, //23 March 2021, UT1-UTC=-0.1709631, Accumulated Leap Seconds=37
    { 2459297.50, 69.354673 }, //24 March 2021, UT1-UTC=-0.1706727, Accumulated Leap Seconds=37
    { 2459298.50, 69.354525 }, //25 March 2021, UT1-UTC=-0.1705246, Accumulated Leap Seconds=37
    { 2459299.50, 69.354584 }, //26 March 2021, UT1-UTC=-0.1705844, Accumulated Leap Seconds=37
    { 2459300.50, 69.354926 }, //27 March 2021, UT1-UTC=-0.1709260, Accumulated Leap Seconds=37
    { 2459301.50, 69.355480 }, //28 March 2021, UT1-UTC=-0.1714805, Accumulated Leap Seconds=37
    { 2459302.50, 69.356252 }, //29 March 2021, UT1-UTC=-0.1722523, Accumulated Leap Seconds=37
    { 2459303.50, 69.357046 }, //30 March 2021, UT1-UTC=-0.1730458, Accumulated Leap Seconds=37
    { 2459304.50, 69.357725 }, //31 March 2021, UT1-UTC=-0.1737247, Accumulated Leap Seconds=37
    { 2459305.50, 69.358222 }, //01 April 2021, UT1-UTC=-0.1742217, Accumulated Leap Seconds=37
    { 2459306.50, 69.358484 }, //02 April 2021, UT1-UTC=-0.1744837, Accumulated Leap Seconds=37
    { 2459307.50, 69.358553 }, //03 April 2021, UT1-UTC=-0.1745526, Accumulated Leap Seconds=37
    { 2459308.50, 69.358477 }, //04 April 2021, UT1-UTC=-0.1744768, Accumulated Leap Seconds=37
    { 2459309.50, 69.358371 }, //05 April 2021, UT1-UTC=-0.1743712, Accumulated Leap Seconds=37
    { 2459310.50, 69.358407 }, //06 April 2021, UT1-UTC=-0.1744072, Accumulated Leap Seconds=37
    { 2459311.50, 69.358601 }, //07 April 2021, UT1-UTC=-0.1746013, Accumulated Leap Seconds=37
    { 2459312.50, 69.358863 }, //08 April 2021, UT1-UTC=-0.1748625, Accumulated Leap Seconds=37
    { 2459313.50, 69.359281 }, //09 April 2021, UT1-UTC=-0.1752811, Accumulated Leap Seconds=37
    { 2459314.50, 69.359755 }, //10 April 2021, UT1-UTC=-0.1757555, Accumulated Leap Seconds=37
    { 2459315.50, 69.360284 }, //11 April 2021, UT1-UTC=-0.1762845, Accumulated Leap Seconds=37
    { 2459316.50, 69.360834 }, //12 April 2021, UT1-UTC=-0.1768340, Accumulated Leap Seconds=37
    { 2459317.50, 69.361252 }, //13 April 2021, UT1-UTC=-0.1772518, Accumulated Leap Seconds=37
    { 2459318.50, 69.361616 }, //14 April 2021, UT1-UTC=-0.1776165, Accumulated Leap Seconds=37
    { 2459319.50, 69.361857 }, //15 April 2021, UT1-UTC=-0.1778568, Accumulated Leap Seconds=37
    { 2459320.50, 69.361991 }, //16 April 2021, UT1-UTC=-0.1779906, Accumulated Leap Seconds=37
    { 2459321.50, 69.362019 }, //17 April 2021, UT1-UTC=-0.1780186, Accumulated Leap Seconds=37
    { 2459322.50, 69.361975 }, //18 April 2021, UT1-UTC=-0.1779747, Accumulated Leap Seconds=37
    { 2459323.50, 69.361868 }, //19 April 2021, UT1-UTC=-0.1778677, Accumulated Leap Seconds=37
    { 2459324.50, 69.361842 }, //20 April 2021, UT1-UTC=-0.1778425, Accumulated Leap Seconds=37
    { 2459325.50, 69.361860 }, //21 April 2021, UT1-UTC=-0.1778604, Accumulated Leap Seconds=37
    { 2459326.50, 69.362044 }, //22 April 2021, UT1-UTC=-0.1780444, Accumulated Leap Seconds=37
    { 2459327.50, 69.362525 }, //23 April 2021, UT1-UTC=-0.1785252, Accumulated Leap Seconds=37
    { 2459328.50, 69.363155 }, //24 April 2021, UT1-UTC=-0.1791550, Accumulated Leap Seconds=37
    { 2459329.50, 69.363962 }, //25 April 2021, UT1-UTC=-0.1799625, Accumulated Leap Seconds=37
    { 2459330.50, 69.364887 }, //26 April 2021, UT1-UTC=-0.1808868, Accumulated Leap Seconds=37
    { 2459331.50, 69.365898 }, //27 April 2021, UT1-UTC=-0.1818984, Accumulated Leap Seconds=37
    { 2459332.50, 69.366703 }, //28 April 2021, UT1-UTC=-0.1827029, Accumulated Leap Seconds=37
    { 2459333.50, 69.367201 }, //29 April 2021, UT1-UTC=-0.1832012, Accumulated Leap Seconds=37
    { 2459334.50, 69.367369 }, //30 April 2021, UT1-UTC=-0.1833693, Accumulated Leap Seconds=37
    { 2459335.50, 69.367300 }, //01 May 2021, UT1-UTC=-0.1833001, Accumulated Leap Seconds=37
    { 2459336.50, 69.367085 }, //02 May 2021, UT1-UTC=-0.1830846, Accumulated Leap Seconds=37
    { 2459337.50, 69.366789 }, //03 May 2021, UT1-UTC=-0.1827895, Accumulated Leap Seconds=37
    { 2459338.50, 69.366519 }, //04 May 2021, UT1-UTC=-0.1825186, Accumulated Leap Seconds=37
    { 2459339.50, 69.366387 }, //05 May 2021, UT1-UTC=-0.1823874, Accumulated Leap Seconds=37
    { 2459340.50, 69.366396 }, //06 May 2021, UT1-UTC=-0.1823958, Accumulated Leap Seconds=37
    { 2459341.50, 69.366557 }, //07 May 2021, UT1-UTC=-0.1825565, Accumulated Leap Seconds=37
    { 2459342.50, 69.366822 }, //08 May 2021, UT1-UTC=-0.1828217, Accumulated Leap Seconds=37
    { 2459343.50, 69.367047 }, //09 May 2021, UT1-UTC=-0.1830469, Accumulated Leap Seconds=37
    { 2459344.50, 69.367223 }, //10 May 2021, UT1-UTC=-0.1832233, Accumulated Leap Seconds=37
    { 2459345.50, 69.367354 }, //11 May 2021, UT1-UTC=-0.1833538, Accumulated Leap Seconds=37
    { 2459346.50, 69.367441 }, //12 May 2021, UT1-UTC=-0.1834414, Accumulated Leap Seconds=37
    { 2459347.50, 69.367301 }, //13 May 2021, UT1-UTC=-0.1833012, Accumulated Leap Seconds=37
    { 2459348.50, 69.367098 }, //14 May 2021, UT1-UTC=-0.1830977, Accumulated Leap Seconds=37
    { 2459349.50, 69.366823 }, //15 May 2021, UT1-UTC=-0.1828230, Accumulated Leap Seconds=37
    { 2459350.50, 69.366535 }, //16 May 2021, UT1-UTC=-0.1825349, Accumulated Leap Seconds=37
    { 2459351.50, 69.366260 }, //17 May 2021, UT1-UTC=-0.1822596, Accumulated Leap Seconds=37
    { 2459352.50, 69.366023 }, //18 May 2021, UT1-UTC=-0.1820229, Accumulated Leap Seconds=37
    { 2459353.50, 69.365971 }, //19 May 2021, UT1-UTC=-0.1819706, Accumulated Leap Seconds=37
    { 2459354.50, 69.366072 }, //20 May 2021, UT1-UTC=-0.1820723, Accumulated Leap Seconds=37
    { 2459355.50, 69.366507 }, //21 May 2021, UT1-UTC=-0.1825070, Accumulated Leap Seconds=37
    { 2459356.50, 69.367026 }, //22 May 2021, UT1-UTC=-0.1830263, Accumulated Leap Seconds=37
    { 2459357.50, 69.367716 }, //23 May 2021, UT1-UTC=-0.1837162, Accumulated Leap Seconds=37
    { 2459358.50, 69.368435 }, //24 May 2021, UT1-UTC=-0.1844354, Accumulated Leap Seconds=37
    { 2459359.50, 69.369041 }, //25 May 2021, UT1-UTC=-0.1850414, Accumulated Leap Seconds=37
    { 2459360.50, 69.369417 }, //26 May 2021, UT1-UTC=-0.1854166, Accumulated Leap Seconds=37
    { 2459361.50, 69.369542 }, //27 May 2021, UT1-UTC=-0.1855424, Accumulated Leap Seconds=37
    { 2459362.50, 69.369404 }, //28 May 2021, UT1-UTC=-0.1854036, Accumulated Leap Seconds=37
    { 2459363.50, 69.369050 }, //29 May 2021, UT1-UTC=-0.1850503, Accumulated Leap Seconds=37
    { 2459364.50, 69.368589 }, //30 May 2021, UT1-UTC=-0.1845894, Accumulated Leap Seconds=37
    { 2459365.50, 69.368161 }, //31 May 2021, UT1-UTC=-0.1841614, Accumulated Leap Seconds=37
    { 2459366.50, 69.367863 }, //01 June 2021, UT1-UTC=-0.1838630, Accumulated Leap Seconds=37
    { 2459367.50, 69.367703 }, //02 June 2021, UT1-UTC=-0.1837035, Accumulated Leap Seconds=37
    { 2459368.50, 69.367660 }, //03 June 2021, UT1-UTC=-0.1836597, Accumulated Leap Seconds=37
    { 2459369.50, 69.367711 }, //04 June 2021, UT1-UTC=-0.1837105, Accumulated Leap Seconds=37
    { 2459370.50, 69.367739 }, //05 June 2021, UT1-UTC=-0.1837394, Accumulated Leap Seconds=37
    { 2459371.50, 69.367666 }, //06 June 2021, UT1-UTC=-0.1836660, Accumulated Leap Seconds=37
    { 2459372.50, 69.367471 }, //07 June 2021, UT1-UTC=-0.1834706, Accumulated Leap Seconds=37
    { 2459373.50, 69.367129 }, //08 June 2021, UT1-UTC=-0.1831291, Accumulated Leap Seconds=37
    { 2459374.50, 69.366574 }, //09 June 2021, UT1-UTC=-0.1825745, Accumulated Leap Seconds=37
    { 2459375.50, 69.365901 }, //10 June 2021, UT1-UTC=-0.1819015, Accumulated Leap Seconds=37
    { 2459376.50, 69.365101 }, //11 June 2021, UT1-UTC=-0.1811011, Accumulated Leap Seconds=37
    { 2459377.50, 69.364170 }, //12 June 2021, UT1-UTC=-0.1801701, Accumulated Leap Seconds=37
    { 2459378.50, 69.363267 }, //13 June 2021, UT1-UTC=-0.1792667, Accumulated Leap Seconds=37
    { 2459379.50, 69.362346 }, //14 June 2021, UT1-UTC=-0.1783462, Accumulated Leap Seconds=37
    { 2459380.50, 69.361572 }, //15 June 2021, UT1-UTC=-0.1775716, Accumulated Leap Seconds=37
    { 2459381.50, 69.360975 }, //16 June 2021, UT1-UTC=-0.1769755, Accumulated Leap Seconds=37
    { 2459382.50, 69.360506 }, //17 June 2021, UT1-UTC=-0.1765064, Accumulated Leap Seconds=37
    { 2459383.50, 69.360282 }, //18 June 2021, UT1-UTC=-0.1762825, Accumulated Leap Seconds=37
    { 2459384.50, 69.360208 }, //19 June 2021, UT1-UTC=-0.1762075, Accumulated Leap Seconds=37
    { 2459385.50, 69.360239 }, //20 June 2021, UT1-UTC=-0.1762386, Accumulated Leap Seconds=37
    { 2459386.50, 69.360206 }, //21 June 2021, UT1-UTC=-0.1762063, Accumulated Leap Seconds=37
    { 2459387.50, 69.359952 }, //22 June 2021, UT1-UTC=-0.1759517, Accumulated Leap Seconds=37
    { 2459388.50, 69.359471 }, //23 June 2021, UT1-UTC=-0.1754708, Accumulated Leap Seconds=37
    { 2459389.50, 69.358663 }, //24 June 2021, UT1-UTC=-0.1746635, Accumulated Leap Seconds=37
    { 2459390.50, 69.357579 }, //25 June 2021, UT1-UTC=-0.1735793, Accumulated Leap Seconds=37
    { 2459391.50, 69.356387 }, //26 June 2021, UT1-UTC=-0.1723866, Accumulated Leap Seconds=37
    { 2459392.50, 69.355140 }, //27 June 2021, UT1-UTC=-0.1711397, Accumulated Leap Seconds=37
    { 2459393.50, 69.353970 }, //28 June 2021, UT1-UTC=-0.1699699, Accumulated Leap Seconds=37
    { 2459394.50, 69.352983 }, //29 June 2021, UT1-UTC=-0.1689826, Accumulated Leap Seconds=37
    { 2459395.50, 69.352193 }, //30 June 2021, UT1-UTC=-0.1681927, Accumulated Leap Seconds=37
    { 2459396.50, 69.351419 }, //01 July 2021, UT1-UTC=-0.1674190, Accumulated Leap Seconds=37
    { 2459397.50, 69.350723 }, //02 July 2021, UT1-UTC=-0.1667234, Accumulated Leap Seconds=37
    { 2459398.50, 69.349989 }, //03 July 2021, UT1-UTC=-0.1659893, Accumulated Leap Seconds=37
    { 2459399.50, 69.349172 }, //04 July 2021, UT1-UTC=-0.1651716, Accumulated Leap Seconds=37
    { 2459400.50, 69.348162 }, //05 July 2021, UT1-UTC=-0.1641619, Accumulated Leap Seconds=37
    { 2459401.50, 69.346978 }, //06 July 2021, UT1-UTC=-0.1629779, Accumulated Leap Seconds=37
    { 2459402.50, 69.345807 }, //07 July 2021, UT1-UTC=-0.1618071, Accumulated Leap Seconds=37
    { 2459403.50, 69.344451 }, //08 July 2021, UT1-UTC=-0.1604507, Accumulated Leap Seconds=37
    { 2459404.50, 69.343025 }, //09 July 2021, UT1-UTC=-0.1590251, Accumulated Leap Seconds=37
    { 2459405.50, 69.341565 }, //10 July 2021, UT1-UTC=-0.1575646, Accumulated Leap Seconds=37
    { 2459406.50, 69.340220 }, //11 July 2021, UT1-UTC=-0.1562195, Accumulated Leap Seconds=37
    { 2459407.50, 69.338968 }, //12 July 2021, UT1-UTC=-0.1549685, Accumulated Leap Seconds=37
    { 2459408.50, 69.337897 }, //13 July 2021, UT1-UTC=-0.1538971, Accumulated Leap Seconds=37
    { 2459409.50, 69.337008 }, //14 July 2021, UT1-UTC=-0.1530082, Accumulated Leap Seconds=37
    { 2459410.50, 69.336398 }, //15 July 2021, UT1-UTC=-0.1523975, Accumulated Leap Seconds=37
    { 2459411.50, 69.335991 }, //16 July 2021, UT1-UTC=-0.1519909, Accumulated Leap Seconds=37
    { 2459412.50, 69.335746 }, //17 July 2021, UT1-UTC=-0.1517456, Accumulated Leap Seconds=37
    { 2459413.50, 69.335531 }, //18 July 2021, UT1-UTC=-0.1515307, Accumulated Leap Seconds=37
    { 2459414.50, 69.335189 }, //19 July 2021, UT1-UTC=-0.1511893, Accumulated Leap Seconds=37
    { 2459415.50, 69.334703 }, //20 July 2021, UT1-UTC=-0.1507034, Accumulated Leap Seconds=37
    { 2459416.50, 69.334077 }, //21 July 2021, UT1-UTC=-0.1500774, Accumulated Leap Seconds=37
    { 2459417.50, 69.333301 }, //22 July 2021, UT1-UTC=-0.1493009, Accumulated Leap Seconds=37
    { 2459418.50, 69.332386 }, //23 July 2021, UT1-UTC=-0.1483860, Accumulated Leap Seconds=37
    { 2459419.50, 69.331474 }, //24 July 2021, UT1-UTC=-0.1474742, Accumulated Leap Seconds=37
    { 2459420.50, 69.330703 }, //25 July 2021, UT1-UTC=-0.1467032, Accumulated Leap Seconds=37
    { 2459421.50, 69.330074 }, //26 July 2021, UT1-UTC=-0.1460740, Accumulated Leap Seconds=37
    { 2459422.50, 69.329535 }, //27 July 2021, UT1-UTC=-0.1455355, Accumulated Leap Seconds=37
    { 2459423.50, 69.329122 }, //28 July 2021, UT1-UTC=-0.1451216, Accumulated Leap Seconds=37
    { 2459424.50, 69.328761 }, //29 July 2021, UT1-UTC=-0.1447606, Accumulated Leap Seconds=37
    { 2459425.50, 69.328366 }, //30 July 2021, UT1-UTC=-0.1443658, Accumulated Leap Seconds=37
    { 2459426.50, 69.327925 }, //31 July 2021, UT1-UTC=-0.1439255, Accumulated Leap Seconds=37
    { 2459427.50, 69.327343 }, //01 August 2021, UT1-UTC=-0.1433427, Accumulated Leap Seconds=37
    { 2459428.50, 69.326648 }, //02 August 2021, UT1-UTC=-0.1426479, Accumulated Leap Seconds=37
    { 2459429.50, 69.325831 }, //03 August 2021, UT1-UTC=-0.1418308, Accumulated Leap Seconds=37
    { 2459430.50, 69.324889 }, //04 August 2021, UT1-UTC=-0.1408895, Accumulated Leap Seconds=37
    { 2459431.50, 69.323859 }, //05 August 2021, UT1-UTC=-0.1398590, Accumulated Leap Seconds=37
    { 2459432.50, 69.322767 }, //06 August 2021, UT1-UTC=-0.1387675, Accumulated Leap Seconds=37
    { 2459433.50, 69.321752 }, //07 August 2021, UT1-UTC=-0.1377522, Accumulated Leap Seconds=37
    { 2459434.50, 69.320744 }, //08 August 2021, UT1-UTC=-0.1367438, Accumulated Leap Seconds=37
    { 2459435.50, 69.319826 }, //09 August 2021, UT1-UTC=-0.1358262, Accumulated Leap Seconds=37
    { 2459436.50, 69.319082 }, //10 August 2021, UT1-UTC=-0.1350824, Accumulated Leap Seconds=37
    { 2459437.50, 69.318512 }, //11 August 2021, UT1-UTC=-0.1345123, Accumulated Leap Seconds=37
    { 2459438.50, 69.318100 }, //12 August 2021, UT1-UTC=-0.1340995, Accumulated Leap Seconds=37
    { 2459439.50, 69.317837 }, //13 August 2021, UT1-UTC=-0.1338370, Accumulated Leap Seconds=37
    { 2459440.50, 69.317510 }, //14 August 2021, UT1-UTC=-0.1335096, Accumulated Leap Seconds=37
    { 2459441.50, 69.317126 }, //15 August 2021, UT1-UTC=-0.1331257, Accumulated Leap Seconds=37
    { 2459442.50, 69.316560 }, //16 August 2021, UT1-UTC=-0.1325605, Accumulated Leap Seconds=37
    { 2459443.50, 69.315756 }, //17 August 2021, UT1-UTC=-0.1317561, Accumulated Leap Seconds=37
    { 2459444.50, 69.314714 }, //18 August 2021, UT1-UTC=-0.1307144, Accumulated Leap Seconds=37
    { 2459445.50, 69.313564 }, //19 August 2021, UT1-UTC=-0.1295644, Accumulated Leap Seconds=37
    { 2459446.50, 69.312361 }, //20 August 2021, UT1-UTC=-0.1283612, Accumulated Leap Seconds=37
    { 2459447.50, 69.311232 }, //21 August 2021, UT1-UTC=-0.1272321, Accumulated Leap Seconds=37
    { 2459448.50, 69.310319 }, //22 August 2021, UT1-UTC=-0.1263191, Accumulated Leap Seconds=37
    { 2459449.50, 69.309574 }, //23 August 2021, UT1-UTC=-0.1255742, Accumulated Leap Seconds=37
    { 2459450.50, 69.308966 }, //24 August 2021, UT1-UTC=-0.1249662, Accumulated Leap Seconds=37
    { 2459451.50, 69.308453 }, //25 August 2021, UT1-UTC=-0.1244534, Accumulated Leap Seconds=37
    { 2459452.50, 69.308009 }, //26 August 2021, UT1-UTC=-0.1240093, Accumulated Leap Seconds=37
    { 2459453.50, 69.307491 }, //27 August 2021, UT1-UTC=-0.1234905, Accumulated Leap Seconds=37
    { 2459454.50, 69.306891 }, //28 August 2021, UT1-UTC=-0.1228912, Accumulated Leap Seconds=37
    { 2459455.50, 69.306107 }, //29 August 2021, UT1-UTC=-0.1221074, Accumulated Leap Seconds=37
    { 2459456.50, 69.305266 }, //30 August 2021, UT1-UTC=-0.1212655, Accumulated Leap Seconds=37
    { 2459457.50, 69.304326 }, //31 August 2021, UT1-UTC=-0.1203261, Accumulated Leap Seconds=37
    { 2459458.50, 69.303330 }, //01 September 2021, UT1-UTC=-0.1193296, Accumulated Leap Seconds=37
    { 2459459.50, 69.302274 }, //02 September 2021, UT1-UTC=-0.1182741, Accumulated Leap Seconds=37
    { 2459460.50, 69.301218 }, //03 September 2021, UT1-UTC=-0.1172182, Accumulated Leap Seconds=37
    { 2459461.50, 69.300339 }, //04 September 2021, UT1-UTC=-0.1163387, Accumulated Leap Seconds=37
    { 2459462.50, 69.299528 }, //05 September 2021, UT1-UTC=-0.1155277, Accumulated Leap Seconds=37
    { 2459463.50, 69.298925 }, //06 September 2021, UT1-UTC=-0.1149250, Accumulated Leap Seconds=37
    { 2459464.50, 69.298567 }, //07 September 2021, UT1-UTC=-0.1145666, Accumulated Leap Seconds=37
    { 2459465.50, 69.298429 }, //08 September 2021, UT1-UTC=-0.1144291, Accumulated Leap Seconds=37
    { 2459466.50, 69.298456 }, //09 September 2021, UT1-UTC=-0.1144561, Accumulated Leap Seconds=37
    { 2459467.50, 69.298497 }, //10 September 2021, UT1-UTC=-0.1144971, Accumulated Leap Seconds=37
    { 2459468.50, 69.298514 }, //11 September 2021, UT1-UTC=-0.1145144, Accumulated Leap Seconds=37
    { 2459469.50, 69.298322 }, //12 September 2021, UT1-UTC=-0.1143216, Accumulated Leap Seconds=37
    { 2459470.50, 69.297908 }, //13 September 2021, UT1-UTC=-0.1139079, Accumulated Leap Seconds=37
    { 2459471.50, 69.297240 }, //14 September 2021, UT1-UTC=-0.1132399, Accumulated Leap Seconds=37
    { 2459472.50, 69.296449 }, //15 September 2021, UT1-UTC=-0.1124489, Accumulated Leap Seconds=37
    { 2459473.50, 69.295575 }, //16 September 2021, UT1-UTC=-0.1115746, Accumulated Leap Seconds=37
    { 2459474.50, 69.294783 }, //17 September 2021, UT1-UTC=-0.1107834, Accumulated Leap Seconds=37
    { 2459475.50, 69.294152 }, //18 September 2021, UT1-UTC=-0.1101516, Accumulated Leap Seconds=37
    { 2459476.50, 69.293794 }, //19 September 2021, UT1-UTC=-0.1097937, Accumulated Leap Seconds=37
    { 2459477.50, 69.293503 }, //20 September 2021, UT1-UTC=-0.1095025, Accumulated Leap Seconds=37
    { 2459478.50, 69.293376 }, //21 September 2021, UT1-UTC=-0.1093763, Accumulated Leap Seconds=37
    { 2459479.50, 69.293244 }, //22 September 2021, UT1-UTC=-0.1092437, Accumulated Leap Seconds=37
    { 2459480.50, 69.293181 }, //23 September 2021, UT1-UTC=-0.1091810, Accumulated Leap Seconds=37
    { 2459481.50, 69.293067 }, //24 September 2021, UT1-UTC=-0.1090666, Accumulated Leap Seconds=37
    { 2459482.50, 69.292850 }, //25 September 2021, UT1-UTC=-0.1088502, Accumulated Leap Seconds=37
    { 2459483.50, 69.292505 }, //26 September 2021, UT1-UTC=-0.1085048, Accumulated Leap Seconds=37
    { 2459484.50, 69.292037 }, //27 September 2021, UT1-UTC=-0.1080366, Accumulated Leap Seconds=37
    { 2459485.50, 69.291398 }, //28 September 2021, UT1-UTC=-0.1073977, Accumulated Leap Seconds=37
    { 2459486.50, 69.290720 }, //29 September 2021, UT1-UTC=-0.1067197, Accumulated Leap Seconds=37
    { 2459487.50, 69.289991 }, //30 September 2021, UT1-UTC=-0.1059910, Accumulated Leap Seconds=37
    { 2459488.50, 69.289249 }, //01 October 2021, UT1-UTC=-0.1052493, Accumulated Leap Seconds=37
    { 2459489.50, 69.288698 }, //02 October 2021, UT1-UTC=-0.1046976, Accumulated Leap Seconds=37
    { 2459490.50, 69.288323 }, //03 October 2021, UT1-UTC=-0.1043230, Accumulated Leap Seconds=37
    { 2459491.50, 69.288115 }, //04 October 2021, UT1-UTC=-0.1041152, Accumulated Leap Seconds=37
    { 2459492.50, 69.288161 }, //05 October 2021, UT1-UTC=-0.1041610, Accumulated Leap Seconds=37
    { 2459493.50, 69.288493 }, //06 October 2021, UT1-UTC=-0.1044929, Accumulated Leap Seconds=37
    { 2459494.50, 69.289040 }, //07 October 2021, UT1-UTC=-0.1050403, Accumulated Leap Seconds=37
    { 2459495.50, 69.289552 }, //08 October 2021, UT1-UTC=-0.1055520, Accumulated Leap Seconds=37
    { 2459496.50, 69.289930 }, //09 October 2021, UT1-UTC=-0.1059297, Accumulated Leap Seconds=37
    { 2459497.50, 69.290053 }, //10 October 2021, UT1-UTC=-0.1060528, Accumulated Leap Seconds=37
    { 2459498.50, 69.289956 }, //11 October 2021, UT1-UTC=-0.1059557, Accumulated Leap Seconds=37
    { 2459499.50, 69.289707 }, //12 October 2021, UT1-UTC=-0.1057066, Accumulated Leap Seconds=37
    { 2459500.50, 69.289372 }, //13 October 2021, UT1-UTC=-0.1053721, Accumulated Leap Seconds=37
    { 2459501.50, 69.289050 }, //14 October 2021, UT1-UTC=-0.1050495, Accumulated Leap Seconds=37
    { 2459502.50, 69.288816 }, //15 October 2021, UT1-UTC=-0.1048160, Accumulated Leap Seconds=37
    { 2459503.50, 69.288714 }, //16 October 2021, UT1-UTC=-0.1047138, Accumulated Leap Seconds=37
    { 2459504.50, 69.288781 }, //17 October 2021, UT1-UTC=-0.1047807, Accumulated Leap Seconds=37
    { 2459505.50, 69.288975 }, //18 October 2021, UT1-UTC=-0.1049746, Accumulated Leap Seconds=37
    { 2459506.50, 69.289321 }, //19 October 2021, UT1-UTC=-0.1053206, Accumulated Leap Seconds=37
    { 2459507.50, 69.289645 }, //20 October 2021, UT1-UTC=-0.1056447, Accumulated Leap Seconds=37
    { 2459508.50, 69.289935 }, //21 October 2021, UT1-UTC=-0.1059345, Accumulated Leap Seconds=37
    { 2459509.50, 69.290146 }, //22 October 2021, UT1-UTC=-0.1061464, Accumulated Leap Seconds=37
    { 2459510.50, 69.290165 }, //23 October 2021, UT1-UTC=-0.1061647, Accumulated Leap Seconds=37
    { 2459511.50, 69.289934 }, //24 October 2021, UT1-UTC=-0.1059344, Accumulated Leap Seconds=37
    { 2459512.50, 69.289563 }, //25 October 2021, UT1-UTC=-0.1055632, Accumulated Leap Seconds=37
    { 2459513.50, 69.289070 }, //26 October 2021, UT1-UTC=-0.1050701, Accumulated Leap Seconds=37
    { 2459514.50, 69.288559 }, //27 October 2021, UT1-UTC=-0.1045588, Accumulated Leap Seconds=37
    { 2459515.50, 69.288128 }, //28 October 2021, UT1-UTC=-0.1041280, Accumulated Leap Seconds=37
    { 2459516.50, 69.287764 }, //29 October 2021, UT1-UTC=-0.1037640, Accumulated Leap Seconds=37
    { 2459517.50, 69.287563 }, //30 October 2021, UT1-UTC=-0.1035626, Accumulated Leap Seconds=37
    { 2459518.50, 69.287638 }, //31 October 2021, UT1-UTC=-0.1036381, Accumulated Leap Seconds=37
    { 2459519.50, 69.288039 }, //01 November 2021, UT1-UTC=-0.1040393, Accumulated Leap Seconds=37
    { 2459520.50, 69.288679 }, //02 November 2021, UT1-UTC=-0.1046788, Accumulated Leap Seconds=37
    { 2459521.50, 69.289524 }, //03 November 2021, UT1-UTC=-0.1055245, Accumulated Leap Seconds=37
    { 2459522.50, 69.290341 }, //04 November 2021, UT1-UTC=-0.1063407, Accumulated Leap Seconds=37
    { 2459523.50, 69.291034 }, //05 November 2021, UT1-UTC=-0.1070342, Accumulated Leap Seconds=37
    { 2459524.50, 69.291421 }, //06 November 2021, UT1-UTC=-0.1074206, Accumulated Leap Seconds=37
    { 2459525.50, 69.291484 }, //07 November 2021, UT1-UTC=-0.1074841, Accumulated Leap Seconds=37
    { 2459526.50, 69.291368 }, //08 November 2021, UT1-UTC=-0.1073681, Accumulated Leap Seconds=37
    { 2459527.50, 69.291173 }, //09 November 2021, UT1-UTC=-0.1071734, Accumulated Leap Seconds=37
    { 2459528.50, 69.290969 }, //10 November 2021, UT1-UTC=-0.1069690, Accumulated Leap Seconds=37
    { 2459529.50, 69.290868 }, //11 November 2021, UT1-UTC=-0.1068676, Accumulated Leap Seconds=37
    { 2459530.50, 69.290989 }, //12 November 2021, UT1-UTC=-0.1069887, Accumulated Leap Seconds=37
    { 2459531.50, 69.291249 }, //13 November 2021, UT1-UTC=-0.1072487, Accumulated Leap Seconds=37
    { 2459532.50, 69.291641 }, //14 November 2021, UT1-UTC=-0.1076407, Accumulated Leap Seconds=37
    { 2459533.50, 69.292005 }, //15 November 2021, UT1-UTC=-0.1080051, Accumulated Leap Seconds=37
    { 2459534.50, 69.292269 }, //16 November 2021, UT1-UTC=-0.1082692, Accumulated Leap Seconds=37
    { 2459535.50, 69.292462 }, //17 November 2021, UT1-UTC=-0.1084617, Accumulated Leap Seconds=37
    { 2459536.50, 69.292531 }, //18 November 2021, UT1-UTC=-0.1085313, Accumulated Leap Seconds=37
    { 2459537.50, 69.292462 }, //19 November 2021, UT1-UTC=-0.1084621, Accumulated Leap Seconds=37
    { 2459538.50, 69.292189 }, //20 November 2021, UT1-UTC=-0.1081889, Accumulated Leap Seconds=37
    { 2459539.50, 69.291770 }, //21 November 2021, UT1-UTC=-0.1077701, Accumulated Leap Seconds=37
    { 2459540.50, 69.291312 }, //22 November 2021, UT1-UTC=-0.1073123, Accumulated Leap Seconds=37
    { 2459541.50, 69.290739 }, //23 November 2021, UT1-UTC=-0.1067389, Accumulated Leap Seconds=37
    { 2459542.50, 69.290211 }, //24 November 2021, UT1-UTC=-0.1062114, Accumulated Leap Seconds=37
    { 2459543.50, 69.289727 }, //25 November 2021, UT1-UTC=-0.1057273, Accumulated Leap Seconds=37
    { 2459544.50, 69.289380 }, //26 November 2021, UT1-UTC=-0.1053797, Accumulated Leap Seconds=37
    { 2459545.50, 69.289317 }, //27 November 2021, UT1-UTC=-0.1053167, Accumulated Leap Seconds=37
    { 2459546.50, 69.289510 }, //28 November 2021, UT1-UTC=-0.1055099, Accumulated Leap Seconds=37
    { 2459547.50, 69.289846 }, //29 November 2021, UT1-UTC=-0.1058461, Accumulated Leap Seconds=37
    { 2459548.50, 69.290250 }, //30 November 2021, UT1-UTC=-0.1062501, Accumulated Leap Seconds=37
    { 2459549.50, 69.290807 }, //01 December 2021, UT1-UTC=-0.1068071, Accumulated Leap Seconds=37
    { 2459550.50, 69.291335 }, //02 December 2021, UT1-UTC=-0.1073353, Accumulated Leap Seconds=37
    { 2459551.50, 69.291760 }, //03 December 2021, UT1-UTC=-0.1077601, Accumulated Leap Seconds=37
    { 2459552.50, 69.291887 }, //04 December 2021, UT1-UTC=-0.1078865, Accumulated Leap Seconds=37
    { 2459553.50, 69.291759 }, //05 December 2021, UT1-UTC=-0.1077593, Accumulated Leap Seconds=37
    { 2459554.50, 69.291523 }, //06 December 2021, UT1-UTC=-0.1075229, Accumulated Leap Seconds=37
    { 2459555.50, 69.291278 }, //07 December 2021, UT1-UTC=-0.1072783, Accumulated Leap Seconds=37
    { 2459556.50, 69.291059 }, //08 December 2021, UT1-UTC=-0.1070588, Accumulated Leap Seconds=37
    { 2459557.50, 69.291089 }, //09 December 2021, UT1-UTC=-0.1070888, Accumulated Leap Seconds=37
    { 2459558.50, 69.291391 }, //10 December 2021, UT1-UTC=-0.1073913, Accumulated Leap Seconds=37
    { 2459559.50, 69.291747 }, //11 December 2021, UT1-UTC=-0.1077469, Accumulated Leap Seconds=37
    { 2459560.50, 69.292188 }, //12 December 2021, UT1-UTC=-0.1081884, Accumulated Leap Seconds=37
    { 2459561.50, 69.292639 }, //13 December 2021, UT1-UTC=-0.1086393, Accumulated Leap Seconds=37
    { 2459562.50, 69.292959 }, //14 December 2021, UT1-UTC=-0.1089589, Accumulated Leap Seconds=37
    { 2459563.50, 69.293194 }, //15 December 2021, UT1-UTC=-0.1091938, Accumulated Leap Seconds=37
    { 2459564.50, 69.293294 }, //16 December 2021, UT1-UTC=-0.1092942, Accumulated Leap Seconds=37
    { 2459565.50, 69.293265 }, //17 December 2021, UT1-UTC=-0.1092654, Accumulated Leap Seconds=37
    { 2459566.50, 69.293035 }, //18 December 2021, UT1-UTC=-0.1090351, Accumulated Leap Seconds=37
    { 2459567.50, 69.292734 }, //19 December 2021, UT1-UTC=-0.1087342, Accumulated Leap Seconds=37
    { 2459568.50, 69.292393 }, //20 December 2021, UT1-UTC=-0.1083927, Accumulated Leap Seconds=37
    { 2459569.50, 69.291998 }, //21 December 2021, UT1-UTC=-0.1079976, Accumulated Leap Seconds=37
    { 2459570.50, 69.291592 }, //22 December 2021, UT1-UTC=-0.1075920, Accumulated Leap Seconds=37
    { 2459571.50, 69.291320 }, //23 December 2021, UT1-UTC=-0.1073200, Accumulated Leap Seconds=37
    { 2459572.50, 69.291254 }, //24 December 2021, UT1-UTC=-0.1072545, Accumulated Leap Seconds=37
    { 2459573.50, 69.291358 }, //25 December 2021, UT1-UTC=-0.1073582, Accumulated Leap Seconds=37
    { 2459574.50, 69.291713 }, //26 December 2021, UT1-UTC=-0.1077127, Accumulated Leap Seconds=37
    { 2459575.50, 69.292291 }, //27 December 2021, UT1-UTC=-0.1082908, Accumulated Leap Seconds=37
    { 2459576.50, 69.292922 }, //28 December 2021, UT1-UTC=-0.1089216, Accumulated Leap Seconds=37
    { 2459577.50, 69.293553 }, //29 December 2021, UT1-UTC=-0.1095526, Accumulated Leap Seconds=37
    { 2459578.50, 69.294094 }, //30 December 2021, UT1-UTC=-0.1100945, Accumulated Leap Seconds=37
    { 2459579.50, 69.294420 }, //31 December 2021, UT1-UTC=-0.1104201, Accumulated Leap Seconds=37
    { 2459580.50, 69.294493 }, //01 January 2022, UT1-UTC=-0.1104932, Accumulated Leap Seconds=37
    { 2459581.50, 69.294361 }, //02 January 2022, UT1-UTC=-0.1103609, Accumulated Leap Seconds=37
    { 2459582.50, 69.294124 }, //03 January 2022, UT1-UTC=-0.1101236, Accumulated Leap Seconds=37
    { 2459583.50, 69.293936 }, //04 January 2022, UT1-UTC=-0.1099363, Accumulated Leap Seconds=37
    { 2459584.50, 69.293852 }, //05 January 2022, UT1-UTC=-0.1098521, Accumulated Leap Seconds=37
    { 2459585.50, 69.294065 }, //06 January 2022, UT1-UTC=-0.1100652, Accumulated Leap Seconds=37
    { 2459586.50, 69.294374 }, //07 January 2022, UT1-UTC=-0.1103744, Accumulated Leap Seconds=37
    { 2459587.50, 69.294775 }, //08 January 2022, UT1-UTC=-0.1107748, Accumulated Leap Seconds=37
    { 2459588.50, 69.295220 }, //09 January 2022, UT1-UTC=-0.1112201, Accumulated Leap Seconds=37
    { 2459589.50, 69.295607 }, //10 January 2022, UT1-UTC=-0.1116067, Accumulated Leap Seconds=37
    { 2459590.50, 69.295865 }, //11 January 2022, UT1-UTC=-0.1118654, Accumulated Leap Seconds=37
    { 2459591.50, 69.295924 }, //12 January 2022, UT1-UTC=-0.1119236, Accumulated Leap Seconds=37
    { 2459592.50, 69.295867 }, //13 January 2022, UT1-UTC=-0.1118666, Accumulated Leap Seconds=37
    { 2459593.50, 69.295607 }, //14 January 2022, UT1-UTC=-0.1116067, Accumulated Leap Seconds=37
    { 2459594.50, 69.295118 }, //15 January 2022, UT1-UTC=-0.1111180, Accumulated Leap Seconds=37
    { 2459595.50, 69.294575 }, //16 January 2022, UT1-UTC=-0.1105751, Accumulated Leap Seconds=37
    { 2459596.50, 69.294000 }, //17 January 2022, UT1-UTC=-0.1099998, Accumulated Leap Seconds=37
    { 2459597.50, 69.293336 }, //18 January 2022, UT1-UTC=-0.1093362, Accumulated Leap Seconds=37
    { 2459598.50, 69.292814 }, //19 January 2022, UT1-UTC=-0.1088136, Accumulated Leap Seconds=37
    { 2459599.50, 69.292420 }, //20 January 2022, UT1-UTC=-0.1084203, Accumulated Leap Seconds=37
    { 2459600.50, 69.292278 }, //21 January 2022, UT1-UTC=-0.1082777, Accumulated Leap Seconds=37
    { 2459601.50, 69.292305 }, //22 January 2022, UT1-UTC=-0.1083049, Accumulated Leap Seconds=37
    { 2459602.50, 69.292483 }, //23 January 2022, UT1-UTC=-0.1084829, Accumulated Leap Seconds=37
    { 2459603.50, 69.292744 }, //24 January 2022, UT1-UTC=-0.1087438, Accumulated Leap Seconds=37
    { 2459604.50, 69.293015 }, //25 January 2022, UT1-UTC=-0.1090147, Accumulated Leap Seconds=37
    { 2459605.50, 69.293203 }, //26 January 2022, UT1-UTC=-0.1092026, Accumulated Leap Seconds=37
    { 2459606.50, 69.293265 }, //27 January 2022, UT1-UTC=-0.1092650, Accumulated Leap Seconds=37
    { 2459607.50, 69.293123 }, //28 January 2022, UT1-UTC=-0.1091235, Accumulated Leap Seconds=37
    { 2459608.50, 69.292804 }, //29 January 2022, UT1-UTC=-0.1088039, Accumulated Leap Seconds=37
    { 2459609.50, 69.292341 }, //30 January 2022, UT1-UTC=-0.1083414, Accumulated Leap Seconds=37
    { 2459610.50, 69.291834 }, //31 January 2022, UT1-UTC=-0.1078345, Accumulated Leap Seconds=37
    { 2459611.50, 69.291395 }, //01 February 2022, UT1-UTC=-0.1073947, Accumulated Leap Seconds=37
    { 2459612.50, 69.291123 }, //02 February 2022, UT1-UTC=-0.1071232, Accumulated Leap Seconds=37
    { 2459613.50, 69.291075 }, //03 February 2022, UT1-UTC=-0.1070753, Accumulated Leap Seconds=37
    { 2459614.50, 69.291219 }, //04 February 2022, UT1-UTC=-0.1072188, Accumulated Leap Seconds=37
    { 2459615.50, 69.291423 }, //05 February 2022, UT1-UTC=-0.1074227, Accumulated Leap Seconds=37
    { 2459616.50, 69.291576 }, //06 February 2022, UT1-UTC=-0.1075762, Accumulated Leap Seconds=37
    { 2459617.50, 69.291661 }, //07 February 2022, UT1-UTC=-0.1076608, Accumulated Leap Seconds=37
    { 2459618.50, 69.291588 }, //08 February 2022, UT1-UTC=-0.1075883, Accumulated Leap Seconds=37
    { 2459619.50, 69.291294 }, //09 February 2022, UT1-UTC=-0.1072943, Accumulated Leap Seconds=37
    { 2459620.50, 69.290813 }, //10 February 2022, UT1-UTC=-0.1068134, Accumulated Leap Seconds=37
    { 2459621.50, 69.290122 }, //11 February 2022, UT1-UTC=-0.1061222, Accumulated Leap Seconds=37
    { 2459622.50, 69.289248 }, //12 February 2022, UT1-UTC=-0.1052485, Accumulated Leap Seconds=37
    { 2459623.50, 69.288413 }, //13 February 2022, UT1-UTC=-0.1044127, Accumulated Leap Seconds=37
    { 2459624.50, 69.287644 }, //14 February 2022, UT1-UTC=-0.1036440, Accumulated Leap Seconds=37
    { 2459625.50, 69.286993 }, //15 February 2022, UT1-UTC=-0.1029930, Accumulated Leap Seconds=37
    { 2459626.50, 69.286541 }, //16 February 2022, UT1-UTC=-0.1025410, Accumulated Leap Seconds=37
    { 2459627.50, 69.286276 }, //17 February 2022, UT1-UTC=-0.1022764, Accumulated Leap Seconds=37
    { 2459628.50, 69.286271 }, //18 February 2022, UT1-UTC=-0.1022710, Accumulated Leap Seconds=37
    { 2459629.50, 69.286465 }, //19 February 2022, UT1-UTC=-0.1024648, Accumulated Leap Seconds=37
    { 2459630.50, 69.286814 }, //20 February 2022, UT1-UTC=-0.1028139, Accumulated Leap Seconds=37
    { 2459631.50, 69.287212 }, //21 February 2022, UT1-UTC=-0.1032117, Accumulated Leap Seconds=37
    { 2459632.50, 69.287512 }, //22 February 2022, UT1-UTC=-0.1035125, Accumulated Leap Seconds=37
    { 2459633.50, 69.287723 }, //23 February 2022, UT1-UTC=-0.1037225, Accumulated Leap Seconds=37
    { 2459634.50, 69.287697 }, //24 February 2022, UT1-UTC=-0.1036969, Accumulated Leap Seconds=37
    { 2459635.50, 69.287470 }, //25 February 2022, UT1-UTC=-0.1034700, Accumulated Leap Seconds=37
    { 2459636.50, 69.287080 }, //26 February 2022, UT1-UTC=-0.1030795, Accumulated Leap Seconds=37
    { 2459637.50, 69.286644 }, //27 February 2022, UT1-UTC=-0.1026443, Accumulated Leap Seconds=37
    { 2459638.50, 69.286326 }, //28 February 2022, UT1-UTC=-0.1023258, Accumulated Leap Seconds=37
    { 2459639.50, 69.286147 }, //01 March 2022, UT1-UTC=-0.1021471, Accumulated Leap Seconds=37
    { 2459640.50, 69.286156 }, //02 March 2022, UT1-UTC=-0.1021557, Accumulated Leap Seconds=37
    { 2459641.50, 69.286457 }, //03 March 2022, UT1-UTC=-0.1024571, Accumulated Leap Seconds=37
    { 2459642.50, 69.286866 }, //04 March 2022, UT1-UTC=-0.1028658, Accumulated Leap Seconds=37
    { 2459643.50, 69.287245 }, //05 March 2022, UT1-UTC=-0.1032450, Accumulated Leap Seconds=37
    { 2459644.50, 69.287543 }, //06 March 2022, UT1-UTC=-0.1035430, Accumulated Leap Seconds=37
    { 2459645.50, 69.287798 }, //07 March 2022, UT1-UTC=-0.1037985, Accumulated Leap Seconds=37
    { 2459646.50, 69.287656 }, //08 March 2022, UT1-UTC=-0.1036561, Accumulated Leap Seconds=37
    { 2459647.50, 69.287367 }, //09 March 2022, UT1-UTC=-0.1033665, Accumulated Leap Seconds=37
    { 2459648.50, 69.286917 }, //10 March 2022, UT1-UTC=-0.1029165, Accumulated Leap Seconds=37
    { 2459649.50, 69.286304 }, //11 March 2022, UT1-UTC=-0.1023040, Accumulated Leap Seconds=37
    { 2459650.50, 69.285652 }, //12 March 2022, UT1-UTC=-0.1016515, Accumulated Leap Seconds=37
    { 2459651.50, 69.285009 }, //13 March 2022, UT1-UTC=-0.1010085, Accumulated Leap Seconds=37
    { 2459652.50, 69.284351 }, //14 March 2022, UT1-UTC=-0.1003511, Accumulated Leap Seconds=37
    { 2459653.50, 69.283798 }, //15 March 2022, UT1-UTC=-0.0997979, Accumulated Leap Seconds=37
    { 2459654.50, 69.283473 }, //16 March 2022, UT1-UTC=-0.0994728, Accumulated Leap Seconds=37
    { 2459655.50, 69.283393 }, //17 March 2022, UT1-UTC=-0.0993932, Accumulated Leap Seconds=37
    { 2459656.50, 69.283522 }, //18 March 2022, UT1-UTC=-0.0995224, Accumulated Leap Seconds=37, Predicted value
    { 2459657.50, 69.283819 }, //19 March 2022, UT1-UTC=-0.0998189, Accumulated Leap Seconds=37, Predicted value
    { 2459658.50, 69.284203 }, //20 March 2022, UT1-UTC=-0.1002028, Accumulated Leap Seconds=37, Predicted value
    { 2459659.50, 69.284543 }, //21 March 2022, UT1-UTC=-0.1005434, Accumulated Leap Seconds=37, Predicted value
    { 2459660.50, 69.284715 }, //22 March 2022, UT1-UTC=-0.1007148, Accumulated Leap Seconds=37, Predicted value
    { 2459661.50, 69.284663 }, //23 March 2022, UT1-UTC=-0.1006626, Accumulated Leap Seconds=37, Predicted value
    { 2459662.50, 69.284398 }, //24 March 2022, UT1-UTC=-0.1003985, Accumulated Leap Seconds=37, Predicted value
    { 2459663.50, 69.283971 }, //25 March 2022, UT1-UTC=-0.0999709, Accumulated Leap Seconds=37, Predicted value
    { 2459664.50, 69.283464 }, //26 March 2022, UT1-UTC=-0.0994642, Accumulated Leap Seconds=37, Predicted value
    { 2459665.50, 69.282981 }, //27 March 2022, UT1-UTC=-0.0989814, Accumulated Leap Seconds=37, Predicted value
    { 2459666.50, 69.282642 }, //28 March 2022, UT1-UTC=-0.0986416, Accumulated Leap Seconds=37, Predicted value
    { 2459667.50, 69.282504 }, //29 March 2022, UT1-UTC=-0.0985039, Accumulated Leap Seconds=37, Predicted value
    { 2459668.50, 69.282582 }, //30 March 2022, UT1-UTC=-0.0985816, Accumulated Leap Seconds=37, Predicted value
    { 2459669.50, 69.282820 }, //31 March 2022, UT1-UTC=-0.0988201, Accumulated Leap Seconds=37, Predicted value
    { 2459670.50, 69.283153 }, //01 April 2022, UT1-UTC=-0.0991533, Accumulated Leap Seconds=37, Predicted value
    { 2459671.50, 69.283471 }, //02 April 2022, UT1-UTC=-0.0994713, Accumulated Leap Seconds=37, Predicted value
    { 2459672.50, 69.283681 }, //03 April 2022, UT1-UTC=-0.0996807, Accumulated Leap Seconds=37, Predicted value
    { 2459673.50, 69.283739 }, //04 April 2022, UT1-UTC=-0.0997387, Accumulated Leap Seconds=37, Predicted value
    { 2459674.50, 69.283614 }, //05 April 2022, UT1-UTC=-0.0996141, Accumulated Leap Seconds=37, Predicted value
    { 2459675.50, 69.283316 }, //06 April 2022, UT1-UTC=-0.0993159, Accumulated Leap Seconds=37, Predicted value
    { 2459676.50, 69.282872 }, //07 April 2022, UT1-UTC=-0.0988719, Accumulated Leap Seconds=37, Predicted value
    { 2459677.50, 69.282338 }, //08 April 2022, UT1-UTC=-0.0983381, Accumulated Leap Seconds=37, Predicted value
    { 2459678.50, 69.281782 }, //09 April 2022, UT1-UTC=-0.0977824, Accumulated Leap Seconds=37, Predicted value
    { 2459679.50, 69.281277 }, //10 April 2022, UT1-UTC=-0.0972773, Accumulated Leap Seconds=37, Predicted value
    { 2459680.50, 69.280890 }, //11 April 2022, UT1-UTC=-0.0968905, Accumulated Leap Seconds=37, Predicted value
    { 2459681.50, 69.280683 }, //12 April 2022, UT1-UTC=-0.0966835, Accumulated Leap Seconds=37, Predicted value
    { 2459682.50, 69.280700 }, //13 April 2022, UT1-UTC=-0.0967000, Accumulated Leap Seconds=37, Predicted value
    { 2459683.50, 69.280953 }, //14 April 2022, UT1-UTC=-0.0969533, Accumulated Leap Seconds=37, Predicted value
    { 2459684.50, 69.281406 }, //15 April 2022, UT1-UTC=-0.0974057, Accumulated Leap Seconds=37, Predicted value
    { 2459685.50, 69.281974 }, //16 April 2022, UT1-UTC=-0.0979742, Accumulated Leap Seconds=37, Predicted value
    { 2459686.50, 69.282543 }, //17 April 2022, UT1-UTC=-0.0985428, Accumulated Leap Seconds=37, Predicted value
    { 2459687.50, 69.282982 }, //18 April 2022, UT1-UTC=-0.0989817, Accumulated Leap Seconds=37, Predicted value
    { 2459688.50, 69.283191 }, //19 April 2022, UT1-UTC=-0.0991912, Accumulated Leap Seconds=37, Predicted value
    { 2459689.50, 69.283138 }, //20 April 2022, UT1-UTC=-0.0991376, Accumulated Leap Seconds=37, Predicted value
    { 2459690.50, 69.282865 }, //21 April 2022, UT1-UTC=-0.0988649, Accumulated Leap Seconds=37, Predicted value
    { 2459691.50, 69.282479 }, //22 April 2022, UT1-UTC=-0.0984791, Accumulated Leap Seconds=37, Predicted value
    { 2459692.50, 69.282103 }, //23 April 2022, UT1-UTC=-0.0981028, Accumulated Leap Seconds=37, Predicted value
    { 2459693.50, 69.281842 }, //24 April 2022, UT1-UTC=-0.0978417, Accumulated Leap Seconds=37, Predicted value
    { 2459694.50, 69.281766 }, //25 April 2022, UT1-UTC=-0.0977656, Accumulated Leap Seconds=37, Predicted value
    { 2459695.50, 69.281894 }, //26 April 2022, UT1-UTC=-0.0978936, Accumulated Leap Seconds=37, Predicted value
    { 2459696.50, 69.282190 }, //27 April 2022, UT1-UTC=-0.0981905, Accumulated Leap Seconds=37, Predicted value
    { 2459697.50, 69.282584 }, //28 April 2022, UT1-UTC=-0.0985836, Accumulated Leap Seconds=37, Predicted value
    { 2459698.50, 69.282985 }, //29 April 2022, UT1-UTC=-0.0989854, Accumulated Leap Seconds=37, Predicted value
    { 2459699.50, 69.283308 }, //30 April 2022, UT1-UTC=-0.0993083, Accumulated Leap Seconds=37, Predicted value
    { 2459700.50, 69.283483 }, //01 May 2022, UT1-UTC=-0.0994834, Accumulated Leap Seconds=37, Predicted value
    { 2459701.50, 69.283471 }, //02 May 2022, UT1-UTC=-0.0994712, Accumulated Leap Seconds=37, Predicted value
    { 2459702.50, 69.283262 }, //03 May 2022, UT1-UTC=-0.0992625, Accumulated Leap Seconds=37, Predicted value
    { 2459703.50, 69.282881 }, //04 May 2022, UT1-UTC=-0.0988806, Accumulated Leap Seconds=37, Predicted value
    { 2459704.50, 69.282371 }, //05 May 2022, UT1-UTC=-0.0983707, Accumulated Leap Seconds=37, Predicted value
    { 2459705.50, 69.281797 }, //06 May 2022, UT1-UTC=-0.0977974, Accumulated Leap Seconds=37, Predicted value
    { 2459706.50, 69.281234 }, //07 May 2022, UT1-UTC=-0.0972340, Accumulated Leap Seconds=37, Predicted value
    { 2459707.50, 69.280752 }, //08 May 2022, UT1-UTC=-0.0967521, Accumulated Leap Seconds=37, Predicted value
    { 2459708.50, 69.280417 }, //09 May 2022, UT1-UTC=-0.0964172, Accumulated Leap Seconds=37, Predicted value
    { 2459709.50, 69.280277 }, //10 May 2022, UT1-UTC=-0.0962773, Accumulated Leap Seconds=37, Predicted value
    { 2459710.50, 69.280360 }, //11 May 2022, UT1-UTC=-0.0963598, Accumulated Leap Seconds=37, Predicted value
    { 2459711.50, 69.280660 }, //12 May 2022, UT1-UTC=-0.0966601, Accumulated Leap Seconds=37, Predicted value
    { 2459712.50, 69.281125 }, //13 May 2022, UT1-UTC=-0.0971254, Accumulated Leap Seconds=37, Predicted value
    { 2459713.50, 69.281653 }, //14 May 2022, UT1-UTC=-0.0976528, Accumulated Leap Seconds=37, Predicted value
    { 2459714.50, 69.282108 }, //15 May 2022, UT1-UTC=-0.0981084, Accumulated Leap Seconds=37, Predicted value
    { 2459715.50, 69.282368 }, //16 May 2022, UT1-UTC=-0.0983679, Accumulated Leap Seconds=37, Predicted value
    { 2459716.50, 69.282359 }, //17 May 2022, UT1-UTC=-0.0983588, Accumulated Leap Seconds=37, Predicted value
    { 2459717.50, 69.282086 }, //18 May 2022, UT1-UTC=-0.0980863, Accumulated Leap Seconds=37, Predicted value
    { 2459718.50, 69.281632 }, //19 May 2022, UT1-UTC=-0.0976323, Accumulated Leap Seconds=37, Predicted value
    { 2459719.50, 69.281123 }, //20 May 2022, UT1-UTC=-0.0971229, Accumulated Leap Seconds=37, Predicted value
    { 2459720.50, 69.280684 }, //21 May 2022, UT1-UTC=-0.0966838, Accumulated Leap Seconds=37, Predicted value
    { 2459721.50, 69.280396 }, //22 May 2022, UT1-UTC=-0.0963959, Accumulated Leap Seconds=37, Predicted value
    { 2459722.50, 69.280285 }, //23 May 2022, UT1-UTC=-0.0962851, Accumulated Leap Seconds=37, Predicted value
    { 2459723.50, 69.280321 }, //24 May 2022, UT1-UTC=-0.0963210, Accumulated Leap Seconds=37, Predicted value
    { 2459724.50, 69.280432 }, //25 May 2022, UT1-UTC=-0.0964322, Accumulated Leap Seconds=37, Predicted value
    { 2459725.50, 69.280534 }, //26 May 2022, UT1-UTC=-0.0965340, Accumulated Leap Seconds=37, Predicted value
    { 2459726.50, 69.280543 }, //27 May 2022, UT1-UTC=-0.0965429, Accumulated Leap Seconds=37, Predicted value
    { 2459727.50, 69.280392 }, //28 May 2022, UT1-UTC=-0.0963919, Accumulated Leap Seconds=37, Predicted value
    { 2459728.50, 69.280038 }, //29 May 2022, UT1-UTC=-0.0960380, Accumulated Leap Seconds=37, Predicted value
    { 2459729.50, 69.279469 }, //30 May 2022, UT1-UTC=-0.0954688, Accumulated Leap Seconds=37, Predicted value
    { 2459730.50, 69.278700 }, //31 May 2022, UT1-UTC=-0.0947000, Accumulated Leap Seconds=37, Predicted value
    { 2459731.50, 69.277769 }, //01 June 2022, UT1-UTC=-0.0937693, Accumulated Leap Seconds=37, Predicted value
    { 2459732.50, 69.276735 }, //02 June 2022, UT1-UTC=-0.0927354, Accumulated Leap Seconds=37, Predicted value
    { 2459733.50, 69.275668 }, //03 June 2022, UT1-UTC=-0.0916680, Accumulated Leap Seconds=37, Predicted value
    { 2459734.50, 69.274642 }, //04 June 2022, UT1-UTC=-0.0906421, Accumulated Leap Seconds=37, Predicted value
    { 2459735.50, 69.273724 }, //05 June 2022, UT1-UTC=-0.0897244, Accumulated Leap Seconds=37, Predicted value
    { 2459736.50, 69.272964 }, //06 June 2022, UT1-UTC=-0.0889638, Accumulated Leap Seconds=37, Predicted value
    { 2459737.50, 69.272389 }, //07 June 2022, UT1-UTC=-0.0883887, Accumulated Leap Seconds=37, Predicted value
    { 2459738.50, 69.272005 }, //08 June 2022, UT1-UTC=-0.0880048, Accumulated Leap Seconds=37, Predicted value
    { 2459739.50, 69.271783 }, //09 June 2022, UT1-UTC=-0.0877833, Accumulated Leap Seconds=37, Predicted value
    { 2459740.50, 69.271656 }, //10 June 2022, UT1-UTC=-0.0876558, Accumulated Leap Seconds=37, Predicted value
    { 2459741.50, 69.271517 }, //11 June 2022, UT1-UTC=-0.0875169, Accumulated Leap Seconds=37, Predicted value
    { 2459742.50, 69.271242 }, //12 June 2022, UT1-UTC=-0.0872421, Accumulated Leap Seconds=37, Predicted value
    { 2459743.50, 69.270731 }, //13 June 2022, UT1-UTC=-0.0867309, Accumulated Leap Seconds=37, Predicted value
    { 2459744.50, 69.269949 }, //14 June 2022, UT1-UTC=-0.0859489, Accumulated Leap Seconds=37, Predicted value
    { 2459745.50, 69.268948 }, //15 June 2022, UT1-UTC=-0.0849481, Accumulated Leap Seconds=37, Predicted value
    { 2459746.50, 69.267848 }, //16 June 2022, UT1-UTC=-0.0838482, Accumulated Leap Seconds=37, Predicted value
    { 2459747.50, 69.266790 }, //17 June 2022, UT1-UTC=-0.0827900, Accumulated Leap Seconds=37, Predicted value
    { 2459748.50, 69.265884 }, //18 June 2022, UT1-UTC=-0.0818840, Accumulated Leap Seconds=37, Predicted value
    { 2459749.50, 69.265173 }, //19 June 2022, UT1-UTC=-0.0811726, Accumulated Leap Seconds=37, Predicted value
    { 2459750.50, 69.264630 }, //20 June 2022, UT1-UTC=-0.0806296, Accumulated Leap Seconds=37, Predicted value
    { 2459751.50, 69.264182 }, //21 June 2022, UT1-UTC=-0.0801821, Accumulated Leap Seconds=37, Predicted value
    { 2459752.50, 69.263737 }, //22 June 2022, UT1-UTC=-0.0797371, Accumulated Leap Seconds=37, Predicted value
    { 2459753.50, 69.263209 }, //23 June 2022, UT1-UTC=-0.0792087, Accumulated Leap Seconds=37, Predicted value
    { 2459754.50, 69.262531 }, //24 June 2022, UT1-UTC=-0.0785309, Accumulated Leap Seconds=37, Predicted value
    { 2459755.50, 69.261662 }, //25 June 2022, UT1-UTC=-0.0776617, Accumulated Leap Seconds=37, Predicted value
    { 2459756.50, 69.260584 }, //26 June 2022, UT1-UTC=-0.0765840, Accumulated Leap Seconds=37, Predicted value
    { 2459757.50, 69.259308 }, //27 June 2022, UT1-UTC=-0.0753078, Accumulated Leap Seconds=37, Predicted value
    { 2459758.50, 69.257867 }, //28 June 2022, UT1-UTC=-0.0738669, Accumulated Leap Seconds=37, Predicted value
    { 2459759.50, 69.256315 }, //29 June 2022, UT1-UTC=-0.0723149, Accumulated Leap Seconds=37, Predicted value
    { 2459760.50, 69.254721 }, //30 June 2022, UT1-UTC=-0.0707212, Accumulated Leap Seconds=37, Predicted value
    { 2459761.50, 69.253160 }, //01 July 2022, UT1-UTC=-0.0691601, Accumulated Leap Seconds=37, Predicted value
    { 2459762.50, 69.251699 }, //02 July 2022, UT1-UTC=-0.0676987, Accumulated Leap Seconds=37, Predicted value
    { 2459763.50, 69.250389 }, //03 July 2022, UT1-UTC=-0.0663894, Accumulated Leap Seconds=37, Predicted value
    { 2459764.50, 69.249236 }, //04 July 2022, UT1-UTC=-0.0652363, Accumulated Leap Seconds=37, Predicted value
    { 2459765.50, 69.248274 }, //05 July 2022, UT1-UTC=-0.0642740, Accumulated Leap Seconds=37, Predicted value
    { 2459766.50, 69.247481 }, //06 July 2022, UT1-UTC=-0.0634809, Accumulated Leap Seconds=37, Predicted value
    { 2459767.50, 69.246804 }, //07 July 2022, UT1-UTC=-0.0628045, Accumulated Leap Seconds=37, Predicted value
    { 2459768.50, 69.246162 }, //08 July 2022, UT1-UTC=-0.0621623, Accumulated Leap Seconds=37, Predicted value
    { 2459769.50, 69.245451 }, //09 July 2022, UT1-UTC=-0.0614513, Accumulated Leap Seconds=37, Predicted value
    { 2459770.50, 69.244572 }, //10 July 2022, UT1-UTC=-0.0605723, Accumulated Leap Seconds=37, Predicted value
    { 2459771.50, 69.243466 }, //11 July 2022, UT1-UTC=-0.0594660, Accumulated Leap Seconds=37, Predicted value
    { 2459772.50, 69.242143 }, //12 July 2022, UT1-UTC=-0.0581433, Accumulated Leap Seconds=37, Predicted value
    { 2459773.50, 69.240693 }, //13 July 2022, UT1-UTC=-0.0566931, Accumulated Leap Seconds=37, Predicted value
    { 2459774.50, 69.239254 }, //14 July 2022, UT1-UTC=-0.0552539, Accumulated Leap Seconds=37, Predicted value
    { 2459775.50, 69.237962 }, //15 July 2022, UT1-UTC=-0.0539620, Accumulated Leap Seconds=37, Predicted value
    { 2459776.50, 69.236899 }, //16 July 2022, UT1-UTC=-0.0528995, Accumulated Leap Seconds=37, Predicted value
    { 2459777.50, 69.236068 }, //17 July 2022, UT1-UTC=-0.0520677, Accumulated Leap Seconds=37, Predicted value
    { 2459778.50, 69.235397 }, //18 July 2022, UT1-UTC=-0.0513970, Accumulated Leap Seconds=37, Predicted value
    { 2459779.50, 69.234779 }, //19 July 2022, UT1-UTC=-0.0507793, Accumulated Leap Seconds=37, Predicted value
    { 2459780.50, 69.234106 }, //20 July 2022, UT1-UTC=-0.0501061, Accumulated Leap Seconds=37, Predicted value
    { 2459781.50, 69.233295 }, //21 July 2022, UT1-UTC=-0.0492955, Accumulated Leap Seconds=37, Predicted value
    { 2459782.50, 69.232301 }, //22 July 2022, UT1-UTC=-0.0483010, Accumulated Leap Seconds=37, Predicted value
    { 2459783.50, 69.231107 }, //23 July 2022, UT1-UTC=-0.0471067, Accumulated Leap Seconds=37, Predicted value
    { 2459784.50, 69.229725 }, //24 July 2022, UT1-UTC=-0.0457246, Accumulated Leap Seconds=37, Predicted value
    { 2459785.50, 69.228188 }, //25 July 2022, UT1-UTC=-0.0441875, Accumulated Leap Seconds=37, Predicted value
    { 2459786.50, 69.226546 }, //26 July 2022, UT1-UTC=-0.0425464, Accumulated Leap Seconds=37, Predicted value
    { 2459787.50, 69.224866 }, //27 July 2022, UT1-UTC=-0.0408662, Accumulated Leap Seconds=37, Predicted value
    { 2459788.50, 69.223220 }, //28 July 2022, UT1-UTC=-0.0392204, Accumulated Leap Seconds=37, Predicted value
    { 2459789.50, 69.221679 }, //29 July 2022, UT1-UTC=-0.0376789, Accumulated Leap Seconds=37, Predicted value
    { 2459790.50, 69.220299 }, //30 July 2022, UT1-UTC=-0.0362992, Accumulated Leap Seconds=37, Predicted value
    { 2459791.50, 69.219117 }, //31 July 2022, UT1-UTC=-0.0351169, Accumulated Leap Seconds=37, Predicted value
    { 2459792.50, 69.218139 }, //01 August 2022, UT1-UTC=-0.0341391, Accumulated Leap Seconds=37, Predicted value
    { 2459793.50, 69.217342 }, //02 August 2022, UT1-UTC=-0.0333419, Accumulated Leap Seconds=37, Predicted value
    { 2459794.50, 69.216673 }, //03 August 2022, UT1-UTC=-0.0326733, Accumulated Leap Seconds=37, Predicted value
    { 2459795.50, 69.216057 }, //04 August 2022, UT1-UTC=-0.0320570, Accumulated Leap Seconds=37, Predicted value
    { 2459796.50, 69.215404 }, //05 August 2022, UT1-UTC=-0.0314038, Accumulated Leap Seconds=37, Predicted value
    { 2459797.50, 69.214626 }, //06 August 2022, UT1-UTC=-0.0306261, Accumulated Leap Seconds=37, Predicted value
    { 2459798.50, 69.213663 }, //07 August 2022, UT1-UTC=-0.0296627, Accumulated Leap Seconds=37, Predicted value
    { 2459799.50, 69.212504 }, //08 August 2022, UT1-UTC=-0.0285039, Accumulated Leap Seconds=37, Predicted value
    { 2459800.50, 69.211207 }, //09 August 2022, UT1-UTC=-0.0272074, Accumulated Leap Seconds=37, Predicted value
    { 2459801.50, 69.209890 }, //10 August 2022, UT1-UTC=-0.0258904, Accumulated Leap Seconds=37, Predicted value
    { 2459802.50, 69.208693 }, //11 August 2022, UT1-UTC=-0.0246931, Accumulated Leap Seconds=37, Predicted value
    { 2459803.50, 69.207729 }, //12 August 2022, UT1-UTC=-0.0237292, Accumulated Leap Seconds=37, Predicted value
    { 2459804.50, 69.207043 }, //13 August 2022, UT1-UTC=-0.0230426, Accumulated Leap Seconds=37, Predicted value
    { 2459805.50, 69.206593 }, //14 August 2022, UT1-UTC=-0.0225933, Accumulated Leap Seconds=37, Predicted value
    { 2459806.50, 69.206278 }, //15 August 2022, UT1-UTC=-0.0222780, Accumulated Leap Seconds=37, Predicted value
    { 2459807.50, 69.205969 }, //16 August 2022, UT1-UTC=-0.0219690, Accumulated Leap Seconds=37, Predicted value
    { 2459808.50, 69.205555 }, //17 August 2022, UT1-UTC=-0.0215552, Accumulated Leap Seconds=37, Predicted value
    { 2459809.50, 69.204966 }, //18 August 2022, UT1-UTC=-0.0209657, Accumulated Leap Seconds=37, Predicted value
    { 2459810.50, 69.204173 }, //19 August 2022, UT1-UTC=-0.0201730, Accumulated Leap Seconds=37, Predicted value
    { 2459811.50, 69.203186 }, //20 August 2022, UT1-UTC=-0.0191855, Accumulated Leap Seconds=37, Predicted value
    { 2459812.50, 69.202037 }, //21 August 2022, UT1-UTC=-0.0180373, Accumulated Leap Seconds=37, Predicted value
    { 2459813.50, 69.200780 }, //22 August 2022, UT1-UTC=-0.0167804, Accumulated Leap Seconds=37, Predicted value
    { 2459814.50, 69.199480 }, //23 August 2022, UT1-UTC=-0.0154803, Accumulated Leap Seconds=37, Predicted value
    { 2459815.50, 69.198210 }, //24 August 2022, UT1-UTC=-0.0142096, Accumulated Leap Seconds=37, Predicted value
    { 2459816.50, 69.197040 }, //25 August 2022, UT1-UTC=-0.0130400, Accumulated Leap Seconds=37, Predicted value
    { 2459817.50, 69.196034 }, //26 August 2022, UT1-UTC=-0.0120338, Accumulated Leap Seconds=37, Predicted value
    { 2459818.50, 69.195234 }, //27 August 2022, UT1-UTC=-0.0112340, Accumulated Leap Seconds=37, Predicted value
    { 2459819.50, 69.194656 }, //28 August 2022, UT1-UTC=-0.0106559, Accumulated Leap Seconds=37, Predicted value
    { 2459820.50, 69.194281 }, //29 August 2022, UT1-UTC=-0.0102808, Accumulated Leap Seconds=37, Predicted value
    { 2459821.50, 69.194055 }, //30 August 2022, UT1-UTC=-0.0100547, Accumulated Leap Seconds=37, Predicted value
    { 2459822.50, 69.193896 }, //31 August 2022, UT1-UTC=-0.0098957, Accumulated Leap Seconds=37, Predicted value
    { 2459823.50, 69.193709 }, //01 September 2022, UT1-UTC=-0.0097088, Accumulated Leap Seconds=37, Predicted value
    { 2459824.50, 69.193406 }, //02 September 2022, UT1-UTC=-0.0094056, Accumulated Leap Seconds=37, Predicted value
    { 2459825.50, 69.192925 }, //03 September 2022, UT1-UTC=-0.0089252, Accumulated Leap Seconds=37, Predicted value
    { 2459826.50, 69.192252 }, //04 September 2022, UT1-UTC=-0.0082519, Accumulated Leap Seconds=37, Predicted value
    { 2459827.50, 69.191428 }, //05 September 2022, UT1-UTC=-0.0074279, Accumulated Leap Seconds=37, Predicted value
    { 2459828.50, 69.190549 }, //06 September 2022, UT1-UTC=-0.0065494, Accumulated Leap Seconds=37, Predicted value
    { 2459829.50, 69.189745 }, //07 September 2022, UT1-UTC=-0.0057452, Accumulated Leap Seconds=37, Predicted value
    { 2459830.50, 69.189138 }, //08 September 2022, UT1-UTC=-0.0051379, Accumulated Leap Seconds=37, Predicted value
    { 2459831.50, 69.188803 }, //09 September 2022, UT1-UTC=-0.0048030, Accumulated Leap Seconds=37, Predicted value
    { 2459832.50, 69.188740 }, //10 September 2022, UT1-UTC=-0.0047401, Accumulated Leap Seconds=37, Predicted value
    { 2459833.50, 69.188873 }, //11 September 2022, UT1-UTC=-0.0048730, Accumulated Leap Seconds=37, Predicted value
    { 2459834.50, 69.189077 }, //12 September 2022, UT1-UTC=-0.0050771, Accumulated Leap Seconds=37, Predicted value
    { 2459835.50, 69.189220 }, //13 September 2022, UT1-UTC=-0.0052202, Accumulated Leap Seconds=37, Predicted value
    { 2459836.50, 69.189199 }, //14 September 2022, UT1-UTC=-0.0051985, Accumulated Leap Seconds=37, Predicted value
    { 2459837.50, 69.188955 }, //15 September 2022, UT1-UTC=-0.0049550, Accumulated Leap Seconds=37, Predicted value
    { 2459838.50, 69.188478 }, //16 September 2022, UT1-UTC=-0.0044784, Accumulated Leap Seconds=37, Predicted value
    { 2459839.50, 69.187793 }, //17 September 2022, UT1-UTC=-0.0037932, Accumulated Leap Seconds=37, Predicted value
    { 2459840.50, 69.186947 }, //18 September 2022, UT1-UTC=-0.0029472, Accumulated Leap Seconds=37, Predicted value
    { 2459841.50, 69.186004 }, //19 September 2022, UT1-UTC=-0.0020036, Accumulated Leap Seconds=37, Predicted value
    { 2459842.50, 69.185033 }, //20 September 2022, UT1-UTC=-0.0010327, Accumulated Leap Seconds=37, Predicted value
    { 2459843.50, 69.184107 }, //21 September 2022, UT1-UTC=-0.0001066, Accumulated Leap Seconds=37, Predicted value
    { 2459844.50, 69.183293 }, //22 September 2022, UT1-UTC= 0.0007072, Accumulated Leap Seconds=37, Predicted value
    { 2459845.50, 69.182647 }, //23 September 2022, UT1-UTC= 0.0013530, Accumulated Leap Seconds=37, Predicted value
    { 2459846.50, 69.182206 }, //24 September 2022, UT1-UTC= 0.0017938, Accumulated Leap Seconds=37, Predicted value
    { 2459847.50, 69.181978 }, //25 September 2022, UT1-UTC= 0.0020222, Accumulated Leap Seconds=37, Predicted value
    { 2459848.50, 69.181930 }, //26 September 2022, UT1-UTC= 0.0020696, Accumulated Leap Seconds=37, Predicted value
    { 2459849.50, 69.181992 }, //27 September 2022, UT1-UTC= 0.0020081, Accumulated Leap Seconds=37, Predicted value
    { 2459850.50, 69.182061 }, //28 September 2022, UT1-UTC= 0.0019394, Accumulated Leap Seconds=37, Predicted value
    { 2459851.50, 69.182030 }, //29 September 2022, UT1-UTC= 0.0019695, Accumulated Leap Seconds=37, Predicted value
    { 2459852.50, 69.181822 }, //30 September 2022, UT1-UTC= 0.0021776, Accumulated Leap Seconds=37, Predicted value
    { 2459853.50, 69.181408 }, //01 October 2022, UT1-UTC= 0.0025918, Accumulated Leap Seconds=37, Predicted value
    { 2459854.50, 69.180821 }, //02 October 2022, UT1-UTC= 0.0031791, Accumulated Leap Seconds=37, Predicted value
    { 2459855.50, 69.180148 }, //03 October 2022, UT1-UTC= 0.0038517, Accumulated Leap Seconds=37, Predicted value
    { 2459856.50, 69.179511 }, //04 October 2022, UT1-UTC= 0.0044894, Accumulated Leap Seconds=37, Predicted value
    { 2459857.50, 69.179030 }, //05 October 2022, UT1-UTC= 0.0049702, Accumulated Leap Seconds=37, Predicted value
    { 2459858.50, 69.178795 }, //06 October 2022, UT1-UTC= 0.0052051, Accumulated Leap Seconds=37, Predicted value
    { 2459859.50, 69.178837 }, //07 October 2022, UT1-UTC= 0.0051635, Accumulated Leap Seconds=37, Predicted value
    { 2459860.50, 69.179117 }, //08 October 2022, UT1-UTC= 0.0048829, Accumulated Leap Seconds=37, Predicted value
    { 2459861.50, 69.179542 }, //09 October 2022, UT1-UTC= 0.0044578, Accumulated Leap Seconds=37, Predicted value
    { 2459862.50, 69.179988 }, //10 October 2022, UT1-UTC= 0.0040116, Accumulated Leap Seconds=37, Predicted value
    { 2459863.50, 69.180339 }, //11 October 2022, UT1-UTC= 0.0036611, Accumulated Leap Seconds=37, Predicted value
    { 2459864.50, 69.180512 }, //12 October 2022, UT1-UTC= 0.0034882, Accumulated Leap Seconds=37, Predicted value
    { 2459865.50, 69.180474 }, //13 October 2022, UT1-UTC= 0.0035256, Accumulated Leap Seconds=37, Predicted value
    { 2459866.50, 69.180240 }, //14 October 2022, UT1-UTC= 0.0037603, Accumulated Leap Seconds=37, Predicted value
    { 2459867.50, 69.179852 }, //15 October 2022, UT1-UTC= 0.0041479, Accumulated Leap Seconds=37, Predicted value
    { 2459868.50, 69.179370 }, //16 October 2022, UT1-UTC= 0.0046301, Accumulated Leap Seconds=37, Predicted value
    { 2459869.50, 69.178855 }, //17 October 2022, UT1-UTC= 0.0051450, Accumulated Leap Seconds=37, Predicted value
    { 2459870.50, 69.178368 }, //18 October 2022, UT1-UTC= 0.0056317, Accumulated Leap Seconds=37, Predicted value
    { 2459871.50, 69.177966 }, //19 October 2022, UT1-UTC= 0.0060336, Accumulated Leap Seconds=37, Predicted value
    { 2459872.50, 69.177700 }, //20 October 2022, UT1-UTC= 0.0063005, Accumulated Leap Seconds=37, Predicted value
    { 2459873.50, 69.177618 }, //21 October 2022, UT1-UTC= 0.0063817, Accumulated Leap Seconds=37, Predicted value
    { 2459874.50, 69.177753 }, //22 October 2022, UT1-UTC= 0.0062474, Accumulated Leap Seconds=37, Predicted value
    { 2459875.50, 69.178080 }, //23 October 2022, UT1-UTC= 0.0059200, Accumulated Leap Seconds=37, Predicted value
    { 2459876.50, 69.178524 }, //24 October 2022, UT1-UTC= 0.0054761, Accumulated Leap Seconds=37, Predicted value
    { 2459877.50, 69.178997 }, //25 October 2022, UT1-UTC= 0.0050033, Accumulated Leap Seconds=37, Predicted value
    { 2459878.50, 69.179409 }, //26 October 2022, UT1-UTC= 0.0045908, Accumulated Leap Seconds=37, Predicted value
    { 2459879.50, 69.179661 }, //27 October 2022, UT1-UTC= 0.0043390, Accumulated Leap Seconds=37, Predicted value
    { 2459880.50, 69.179647 }, //28 October 2022, UT1-UTC= 0.0043528, Accumulated Leap Seconds=37, Predicted value
    { 2459881.50, 69.179440 }, //29 October 2022, UT1-UTC= 0.0045600, Accumulated Leap Seconds=37, Predicted value
    { 2459882.50, 69.179147 }, //30 October 2022, UT1-UTC= 0.0048525, Accumulated Leap Seconds=37, Predicted value
    { 2459883.50, 69.178839 }, //31 October 2022, UT1-UTC= 0.0051611, Accumulated Leap Seconds=37, Predicted value
    { 2459884.50, 69.178689 }, //01 November 2022, UT1-UTC= 0.0053112, Accumulated Leap Seconds=37, Predicted value
    { 2459885.50, 69.178765 }, //02 November 2022, UT1-UTC= 0.0052351, Accumulated Leap Seconds=37, Predicted value
    { 2459886.50, 69.179078 }, //03 November 2022, UT1-UTC= 0.0049218, Accumulated Leap Seconds=37, Predicted value
    { 2459887.50, 69.179560 }, //04 November 2022, UT1-UTC= 0.0044400, Accumulated Leap Seconds=37, Predicted value
    { 2459888.50, 69.180251 }, //05 November 2022, UT1-UTC= 0.0037490, Accumulated Leap Seconds=37, Predicted value
    { 2459889.50, 69.180939 }, //06 November 2022, UT1-UTC= 0.0030611, Accumulated Leap Seconds=37, Predicted value
    { 2459890.50, 69.181591 }, //07 November 2022, UT1-UTC= 0.0024090, Accumulated Leap Seconds=37, Predicted value
    { 2459891.50, 69.182143 }, //08 November 2022, UT1-UTC= 0.0018566, Accumulated Leap Seconds=37, Predicted value
    { 2459892.50, 69.182537 }, //09 November 2022, UT1-UTC= 0.0014630, Accumulated Leap Seconds=37, Predicted value
    { 2459893.50, 69.182797 }, //10 November 2022, UT1-UTC= 0.0012029, Accumulated Leap Seconds=37, Predicted value
    { 2459894.50, 69.182963 }, //11 November 2022, UT1-UTC= 0.0010371, Accumulated Leap Seconds=37, Predicted value
    { 2459895.50, 69.182941 }, //12 November 2022, UT1-UTC= 0.0010585, Accumulated Leap Seconds=37, Predicted value
    { 2459896.50, 69.182871 }, //13 November 2022, UT1-UTC= 0.0011294, Accumulated Leap Seconds=37, Predicted value
    { 2459897.50, 69.182799 }, //14 November 2022, UT1-UTC= 0.0012007, Accumulated Leap Seconds=37, Predicted value
    { 2459898.50, 69.182798 }, //15 November 2022, UT1-UTC= 0.0012019, Accumulated Leap Seconds=37, Predicted value
    { 2459899.50, 69.182908 }, //16 November 2022, UT1-UTC= 0.0010917, Accumulated Leap Seconds=37, Predicted value
    { 2459900.50, 69.183166 }, //17 November 2022, UT1-UTC= 0.0008344, Accumulated Leap Seconds=37, Predicted value
    { 2459901.50, 69.183645 }, //18 November 2022, UT1-UTC= 0.0003554, Accumulated Leap Seconds=37, Predicted value
    { 2459902.50, 69.184284 }, //19 November 2022, UT1-UTC=-0.0002839, Accumulated Leap Seconds=37, Predicted value
    { 2459903.50, 69.185091 }, //20 November 2022, UT1-UTC=-0.0010908, Accumulated Leap Seconds=37, Predicted value
    { 2459904.50, 69.185971 }, //21 November 2022, UT1-UTC=-0.0019715, Accumulated Leap Seconds=37, Predicted value
    { 2459905.50, 69.186816 }, //22 November 2022, UT1-UTC=-0.0028160, Accumulated Leap Seconds=37, Predicted value
    { 2459906.50, 69.187592 }, //23 November 2022, UT1-UTC=-0.0035923, Accumulated Leap Seconds=37, Predicted value
    { 2459907.50, 69.188170 }, //24 November 2022, UT1-UTC=-0.0041703, Accumulated Leap Seconds=37, Predicted value
    { 2459908.50, 69.188480 }, //25 November 2022, UT1-UTC=-0.0044802, Accumulated Leap Seconds=37, Predicted value
    { 2459909.50, 69.188619 }, //26 November 2022, UT1-UTC=-0.0046185, Accumulated Leap Seconds=37, Predicted value
    { 2459910.50, 69.188728 }, //27 November 2022, UT1-UTC=-0.0047279, Accumulated Leap Seconds=37, Predicted value
    { 2459911.50, 69.188863 }, //28 November 2022, UT1-UTC=-0.0048635, Accumulated Leap Seconds=37, Predicted value
    { 2459912.50, 69.189086 }, //29 November 2022, UT1-UTC=-0.0050855, Accumulated Leap Seconds=37, Predicted value
    { 2459913.50, 69.189476 }, //30 November 2022, UT1-UTC=-0.0054756, Accumulated Leap Seconds=37, Predicted value
    { 2459914.50, 69.190079 }, //01 December 2022, UT1-UTC=-0.0060792, Accumulated Leap Seconds=37, Predicted value
    { 2459915.50, 69.190776 }, //02 December 2022, UT1-UTC=-0.0067755, Accumulated Leap Seconds=37, Predicted value
    { 2459916.50, 69.191438 }, //03 December 2022, UT1-UTC=-0.0074382, Accumulated Leap Seconds=37, Predicted value
    { 2459917.50, 69.192002 }, //04 December 2022, UT1-UTC=-0.0080017, Accumulated Leap Seconds=37, Predicted value
    { 2459918.50, 69.192349 }, //05 December 2022, UT1-UTC=-0.0083493, Accumulated Leap Seconds=37, Predicted value
    { 2459919.50, 69.192523 }, //06 December 2022, UT1-UTC=-0.0085235, Accumulated Leap Seconds=37, Predicted value
    { 2459920.50, 69.192473 }, //07 December 2022, UT1-UTC=-0.0084729, Accumulated Leap Seconds=37, Predicted value
    { 2459921.50, 69.192236 }, //08 December 2022, UT1-UTC=-0.0082361, Accumulated Leap Seconds=37, Predicted value
    { 2459922.50, 69.191761 }, //09 December 2022, UT1-UTC=-0.0077608, Accumulated Leap Seconds=37, Predicted value
    { 2459923.50, 69.191188 }, //10 December 2022, UT1-UTC=-0.0071879, Accumulated Leap Seconds=37, Predicted value
    { 2459924.50, 69.190565 }, //11 December 2022, UT1-UTC=-0.0065650, Accumulated Leap Seconds=37, Predicted value
    { 2459925.50, 69.190041 }, //12 December 2022, UT1-UTC=-0.0060407, Accumulated Leap Seconds=37, Predicted value
    { 2459926.50, 69.189460 }, //13 December 2022, UT1-UTC=-0.0054597, Accumulated Leap Seconds=37, Predicted value
    { 2459927.50, 69.189057 }, //14 December 2022, UT1-UTC=-0.0050566, Accumulated Leap Seconds=37, Predicted value
    { 2459928.50, 69.188893 }, //15 December 2022, UT1-UTC=-0.0048931, Accumulated Leap Seconds=37, Predicted value
    { 2459929.50, 69.188897 }, //16 December 2022, UT1-UTC=-0.0048972, Accumulated Leap Seconds=37, Predicted value
    { 2459930.50, 69.189047 }, //17 December 2022, UT1-UTC=-0.0050472, Accumulated Leap Seconds=37, Predicted value
    { 2459931.50, 69.189230 }, //18 December 2022, UT1-UTC=-0.0052304, Accumulated Leap Seconds=37, Predicted value
    { 2459932.50, 69.189486 }, //19 December 2022, UT1-UTC=-0.0054863, Accumulated Leap Seconds=37, Predicted value
    { 2459933.50, 69.189583 }, //20 December 2022, UT1-UTC=-0.0055827, Accumulated Leap Seconds=37, Predicted value
    { 2459934.50, 69.189462 }, //21 December 2022, UT1-UTC=-0.0054618, Accumulated Leap Seconds=37, Predicted value
    { 2459935.50, 69.189151 }, //22 December 2022, UT1-UTC=-0.0051514, Accumulated Leap Seconds=37, Predicted value
    { 2459936.50, 69.188724 }, //23 December 2022, UT1-UTC=-0.0047237, Accumulated Leap Seconds=37, Predicted value
    { 2459937.50, 69.188213 }, //24 December 2022, UT1-UTC=-0.0042131, Accumulated Leap Seconds=37, Predicted value
    { 2459938.50, 69.187815 }, //25 December 2022, UT1-UTC=-0.0038154, Accumulated Leap Seconds=37, Predicted value
    { 2459939.50, 69.187647 }, //26 December 2022, UT1-UTC=-0.0036475, Accumulated Leap Seconds=37, Predicted value
    { 2459940.50, 69.187695 }, //27 December 2022, UT1-UTC=-0.0036951, Accumulated Leap Seconds=37, Predicted value
    { 2459941.50, 69.188032 }, //28 December 2022, UT1-UTC=-0.0040323, Accumulated Leap Seconds=37, Predicted value
    { 2459942.50, 69.188549 }, //29 December 2022, UT1-UTC=-0.0045494, Accumulated Leap Seconds=37, Predicted value
    { 2459943.50, 69.189089 }, //30 December 2022, UT1-UTC=-0.0050887, Accumulated Leap Seconds=37, Predicted value
    { 2459944.50, 69.189585 }, //31 December 2022, UT1-UTC=-0.0055851, Accumulated Leap Seconds=37, Predicted value
    { 2459945.50, 69.189898 }, //01 January 2023, UT1-UTC=-0.0058978, Accumulated Leap Seconds=37, Predicted value
    { 2459946.50, 69.190091 }, //02 January 2023, UT1-UTC=-0.0060914, Accumulated Leap Seconds=37, Predicted value
    { 2459947.50, 69.190062 }, //03 January 2023, UT1-UTC=-0.0060623, Accumulated Leap Seconds=37, Predicted value
    { 2459948.50, 69.189799 }, //04 January 2023, UT1-UTC=-0.0057991, Accumulated Leap Seconds=37, Predicted value
    { 2459949.50, 69.189434 }, //05 January 2023, UT1-UTC=-0.0054336, Accumulated Leap Seconds=37, Predicted value
    { 2459950.50, 69.188907 }, //06 January 2023, UT1-UTC=-0.0049071, Accumulated Leap Seconds=37, Predicted value
    { 2459951.50, 69.188384 }, //07 January 2023, UT1-UTC=-0.0043839, Accumulated Leap Seconds=37, Predicted value
    { 2459952.50, 69.187887 }, //08 January 2023, UT1-UTC=-0.0038868, Accumulated Leap Seconds=37, Predicted value
    { 2459953.50, 69.187448 }, //09 January 2023, UT1-UTC=-0.0034478, Accumulated Leap Seconds=37, Predicted value
    { 2459954.50, 69.187210 }, //10 January 2023, UT1-UTC=-0.0032097, Accumulated Leap Seconds=37, Predicted value
    { 2459955.50, 69.187152 }, //11 January 2023, UT1-UTC=-0.0031518, Accumulated Leap Seconds=37, Predicted value
    { 2459956.50, 69.187287 }, //12 January 2023, UT1-UTC=-0.0032870, Accumulated Leap Seconds=37, Predicted value
    { 2459957.50, 69.187619 }, //13 January 2023, UT1-UTC=-0.0036190, Accumulated Leap Seconds=37, Predicted value
    { 2459958.50, 69.188074 }, //14 January 2023, UT1-UTC=-0.0040739, Accumulated Leap Seconds=37, Predicted value
    { 2459959.50, 69.188521 }, //15 January 2023, UT1-UTC=-0.0045215, Accumulated Leap Seconds=37, Predicted value
    { 2459960.50, 69.188904 }, //16 January 2023, UT1-UTC=-0.0049036, Accumulated Leap Seconds=37, Predicted value
    { 2459961.50, 69.189138 }, //17 January 2023, UT1-UTC=-0.0051382, Accumulated Leap Seconds=37, Predicted value
    { 2459962.50, 69.189164 }, //18 January 2023, UT1-UTC=-0.0051639, Accumulated Leap Seconds=37, Predicted value
    { 2459963.50, 69.189012 }, //19 January 2023, UT1-UTC=-0.0050120, Accumulated Leap Seconds=37, Predicted value
    { 2459964.50, 69.188785 }, //20 January 2023, UT1-UTC=-0.0047853, Accumulated Leap Seconds=37, Predicted value
    { 2459965.50, 69.188596 }, //21 January 2023, UT1-UTC=-0.0045964, Accumulated Leap Seconds=37, Predicted value
    { 2459966.50, 69.188547 }, //22 January 2023, UT1-UTC=-0.0045470, Accumulated Leap Seconds=37, Predicted value
    { 2459967.50, 69.188706 }, //23 January 2023, UT1-UTC=-0.0047056, Accumulated Leap Seconds=37, Predicted value
    { 2459968.50, 69.189143 }, //24 January 2023, UT1-UTC=-0.0051434, Accumulated Leap Seconds=37, Predicted value
    { 2459969.50, 69.189811 }, //25 January 2023, UT1-UTC=-0.0058112, Accumulated Leap Seconds=37, Predicted value
    { 2459970.50, 69.190595 }, //26 January 2023, UT1-UTC=-0.0065954, Accumulated Leap Seconds=37, Predicted value
    { 2459971.50, 69.191341 }, //27 January 2023, UT1-UTC=-0.0073410, Accumulated Leap Seconds=37, Predicted value
    { 2459972.50, 69.192088 }, //28 January 2023, UT1-UTC=-0.0080883, Accumulated Leap Seconds=37, Predicted value
    { 2459973.50, 69.192529 }, //29 January 2023, UT1-UTC=-0.0085287, Accumulated Leap Seconds=37, Predicted value
    { 2459974.50, 69.192783 }, //30 January 2023, UT1-UTC=-0.0087826, Accumulated Leap Seconds=37, Predicted value
    { 2459975.50, 69.192774 }, //31 January 2023, UT1-UTC=-0.0087735, Accumulated Leap Seconds=37, Predicted value
    { 2459976.50, 69.192617 }, //01 February 2023, UT1-UTC=-0.0086169, Accumulated Leap Seconds=37, Predicted value
    { 2459977.50, 69.192325 }, //02 February 2023, UT1-UTC=-0.0083253, Accumulated Leap Seconds=37, Predicted value
    { 2459978.50, 69.191929 }, //03 February 2023, UT1-UTC=-0.0079286, Accumulated Leap Seconds=37, Predicted value
    { 2459979.50, 69.191506 }, //04 February 2023, UT1-UTC=-0.0075057, Accumulated Leap Seconds=37, Predicted value
    { 2459980.50, 69.191085 }, //05 February 2023, UT1-UTC=-0.0070855, Accumulated Leap Seconds=37, Predicted value
    { 2459981.50, 69.190879 }, //06 February 2023, UT1-UTC=-0.0068795, Accumulated Leap Seconds=37, Predicted value
    { 2459982.50, 69.190751 }, //07 February 2023, UT1-UTC=-0.0067512, Accumulated Leap Seconds=37, Predicted value
    { 2459983.50, 69.190744 }, //08 February 2023, UT1-UTC=-0.0067444, Accumulated Leap Seconds=37, Predicted value
    { 2459984.50, 69.190887 }, //09 February 2023, UT1-UTC=-0.0068872, Accumulated Leap Seconds=37, Predicted value
    { 2459985.50, 69.191177 }, //10 February 2023, UT1-UTC=-0.0071770, Accumulated Leap Seconds=37, Predicted value
    { 2459986.50, 69.191456 }, //11 February 2023, UT1-UTC=-0.0074558, Accumulated Leap Seconds=37, Predicted value
    { 2459987.50, 69.191643 }, //12 February 2023, UT1-UTC=-0.0076427, Accumulated Leap Seconds=37, Predicted value
    { 2459988.50, 69.191700 }, //13 February 2023, UT1-UTC=-0.0077005, Accumulated Leap Seconds=37, Predicted value
    { 2459989.50, 69.191568 }, //14 February 2023, UT1-UTC=-0.0075683, Accumulated Leap Seconds=37, Predicted value
    { 2459990.50, 69.191301 }, //15 February 2023, UT1-UTC=-0.0073008, Accumulated Leap Seconds=37, Predicted value
    { 2459991.50, 69.190958 }, //16 February 2023, UT1-UTC=-0.0069581, Accumulated Leap Seconds=37, Predicted value
    { 2459992.50, 69.190596 }, //17 February 2023, UT1-UTC=-0.0065960, Accumulated Leap Seconds=37, Predicted value
    { 2459993.50, 69.190337 }, //18 February 2023, UT1-UTC=-0.0063371, Accumulated Leap Seconds=37, Predicted value
    { 2459994.50, 69.190344 }, //19 February 2023, UT1-UTC=-0.0063438, Accumulated Leap Seconds=37, Predicted value
    { 2459995.50, 69.190681 }, //20 February 2023, UT1-UTC=-0.0066811, Accumulated Leap Seconds=37, Predicted value
    { 2459996.50, 69.191332 }, //21 February 2023, UT1-UTC=-0.0073319, Accumulated Leap Seconds=37, Predicted value
    { 2459997.50, 69.192052 }, //22 February 2023, UT1-UTC=-0.0080519, Accumulated Leap Seconds=37, Predicted value
    { 2459998.50, 69.192817 }, //23 February 2023, UT1-UTC=-0.0088174, Accumulated Leap Seconds=37, Predicted value
    { 2459999.50, 69.193531 }, //24 February 2023, UT1-UTC=-0.0095311, Accumulated Leap Seconds=37, Predicted value
    { 2460000.50, 69.194159 }, //25 February 2023, UT1-UTC=-0.0101595, Accumulated Leap Seconds=37, Predicted value
    { 2460001.50, 69.194486 }, //26 February 2023, UT1-UTC=-0.0104863, Accumulated Leap Seconds=37, Predicted value
    { 2460002.50, 69.194585 }, //27 February 2023, UT1-UTC=-0.0105853, Accumulated Leap Seconds=37, Predicted value
    { 2460003.50, 69.194543 }, //28 February 2023, UT1-UTC=-0.0105427, Accumulated Leap Seconds=37, Predicted value
    { 2460004.50, 69.194305 }, //01 March 2023, UT1-UTC=-0.0103049, Accumulated Leap Seconds=37, Predicted value
    { 2460005.50, 69.194021 }, //02 March 2023, UT1-UTC=-0.0100207, Accumulated Leap Seconds=37, Predicted value
    { 2460006.50, 69.193719 }, //03 March 2023, UT1-UTC=-0.0097187, Accumulated Leap Seconds=37, Predicted value
    { 2460007.50, 69.193466 }, //04 March 2023, UT1-UTC=-0.0094663, Accumulated Leap Seconds=37, Predicted value
    { 2460008.50, 69.193308 }, //05 March 2023, UT1-UTC=-0.0093079, Accumulated Leap Seconds=37, Predicted value
    { 2460009.50, 69.193310 }, //06 March 2023, UT1-UTC=-0.0093100, Accumulated Leap Seconds=37, Predicted value
    { 2460010.50, 69.193446 }, //07 March 2023, UT1-UTC=-0.0094459, Accumulated Leap Seconds=37, Predicted value
    { 2460011.50, 69.193825 }, //08 March 2023, UT1-UTC=-0.0098253, Accumulated Leap Seconds=37, Predicted value
    { 2460012.50, 69.194306 }, //09 March 2023, UT1-UTC=-0.0103065, Accumulated Leap Seconds=37, Predicted value
    { 2460013.50, 69.194842 }, //10 March 2023, UT1-UTC=-0.0108423, Accumulated Leap Seconds=37, Predicted value
    { 2460014.50, 69.195397 }, //11 March 2023, UT1-UTC=-0.0113972, Accumulated Leap Seconds=37, Predicted value
    { 2460015.50, 69.195819 }, //12 March 2023, UT1-UTC=-0.0118193, Accumulated Leap Seconds=37, Predicted value
    { 2460016.50, 69.196140 }, //13 March 2023, UT1-UTC=-0.0121404, Accumulated Leap Seconds=37, Predicted value
    { 2460017.50, 69.196213 }, //14 March 2023, UT1-UTC=-0.0122130, Accumulated Leap Seconds=37, Predicted value
    { 2460018.50, 69.196011 }, //15 March 2023, UT1-UTC=-0.0120110, Accumulated Leap Seconds=37, Predicted value
    { 2460019.50, 69.195689 }, //16 March 2023, UT1-UTC=-0.0116890, Accumulated Leap Seconds=37, Predicted value
    { 2460020.50, 69.195380 }, //17 March 2023, UT1-UTC=-0.0113799, Accumulated Leap Seconds=37, Predicted value
    { 2460021.50, 69.195185 }, //18 March 2023, UT1-UTC=-0.0111847, Accumulated Leap Seconds=37, Predicted value
    { 2460022.50, 69.195223 }, //19 March 2023, UT1-UTC=-0.0112226, Accumulated Leap Seconds=37, Predicted value
    { 2460023.50, 69.195522 }, //20 March 2023, UT1-UTC=-0.0115223, Accumulated Leap Seconds=37, Predicted value
    { 2460024.50, 69.196079 }, //21 March 2023, UT1-UTC=-0.0120794, Accumulated Leap Seconds=37, Predicted value
    { 2460025.50, 69.196783 }, //22 March 2023, UT1-UTC=-0.0127835, Accumulated Leap Seconds=37, Predicted value
    { 2460026.50, 69.197422 }, //23 March 2023, UT1-UTC=-0.0134220, Accumulated Leap Seconds=37, Predicted value
    { 2460027.50, 69.197947 }, //24 March 2023, UT1-UTC=-0.0139467, Accumulated Leap Seconds=37, Predicted value
    { 2460028.50, 69.198219 }, //25 March 2023, UT1-UTC=-0.0142187, Accumulated Leap Seconds=37, Predicted value

  //Values from https://cddis.nasa.gov/archive/products/iers/deltat.preds
    { 2460036.75, 69.82    }, //2023.25, Predicted value
    { 2460128.00, 69.87    }, //2023.50, Predicted value
    { 2460219.25, 69.89    }, //2023.75, Predicted value
    { 2460310.50, 69.97    }, //2024.00, Predicted value
    { 2460402.00, 70.05    }, //2024.25, Predicted value
    { 2460493.50, 70.11    }, //2024.50, Predicted value
    { 2460585.00, 70.12    }, //2024.75, Predicted value
    { 2460676.50, 70.20    }, //2025.00, Predicted value
    { 2460767.75, 70.29    }, //2025.25, Predicted value
    { 2460859.00, 70.34    }, //2025.50, Predicted value
    { 2460950.25, 70.35    }, //2025.75, Predicted value
    { 2461041.50, 70.43    }, //2026.00, Predicted value
    { 2461132.75, 70.52    }, //2026.25, Predicted value
    { 2461224.00, 70.57    }, //2026.50, Predicted value
    { 2461315.25, 70.59    }, //2026.75, Predicted value
    { 2461406.50, 70.67    }, //2027.00, Predicted value
    { 2461497.75, 70.75    }, //2027.25, Predicted value
    { 2461589.00, 70.81    }, //2027.50, Predicted value
    { 2461680.25, 70.83    }, //2027.75, Predicted value
    { 2461771.50, 70.91    }, //2028.00, Predicted value
    { 2461863.00, 71.00    }, //2028.25, Predicted value
    { 2461954.50, 71.07    }, //2028.50, Predicted value
    { 2462046.00, 71.09    }, //2028.75, Predicted value
    { 2462137.50, 71.18    }, //2029.00, Predicted value
    { 2462228.75, 71.27    }, //2029.25, Predicted value
    { 2462320.00, 71.34    }, //2029.50, Predicted value
    { 2462411.25, 71.36    }, //2029.75, Predicted value
    { 2462502.50, 71.46    }, //2030.00, Predicted value
    { 2462593.75, 71.56    }, //2030.25, Predicted value
    { 2462685.00, 71.63    }, //2030.50, Predicted value
    { 2462776.25, 71.66    }, //2030.75, Predicted value
    { 2462867.50, 71.76    }, //2031.00, Predicted value
    { 2462958.75, 71.86    }, //2031.25, Predicted value
    { 2463050.00, 71.94    }, //2031.50, Predicted value
    { 2463141.25, 71.97    }, //2031.75, Predicted value
    { 2463232.50, 72.07    }  //2032.00, Predicted value
    
    
    
////All these final values are predicted values from Year 2019.75 to 2027.75 are taken from http://maia.usno.navy.mil/ser7/deltat.preds
//  { 2458758.5,  69.71   }, //2019.75
//  { 2458849.5,  69.87   }, //2020.00
//  { 2458940.5,  70.03   }, //2020.25
//  { 2459032.5,  70.16   }, //2020.50
//  { 2459123.5,  70.24   }, //2020.75
//  { 2459214.5,  70.39   }, //2021.00
//  { 2459306.5,  70.55   }, //2021.25
//  { 2459397.5,  70.68   }, //2021.50
//  { 2459488.5,  70.76   }, //2021.75
//  { 2459580.5,  70.91   }, //2022.00
//  { 2459671.5,  71.06   }, //2022.25
//  { 2459762.5,  71.18   }, //2022.50
//  { 2459853.5,  71.25   }, //2022.75
//  { 2459945.5,  71.40   }, //2023.00
//  { 2460036.5,  71.54   }, //2023.25
//  { 2460127.5,  71.67   }, //2023.50
//  { 2460219.5,  71.74   }, //2023.75
//  { 2460310.5,  71.88   }, //2024.00
//  { 2460401.5,  72.03   }, //2024.25
//  { 2460493.5,  72.15   }, //2024.50
//  { 2460584.5,  72.22   }, //2024.75
//  { 2460675.5,  72.36   }, //2025.00
//  { 2460767.5,  72.50   }, //2025.25
//  { 2460858.5,  72.62   }, //2025.50
//  { 2460949.5,  72.69   }, //2025.75
//  { 2461041.5,  72.83   }, //2026.00
//  { 2461132.5,  72.98   }, //2026.25
//  { 2461223.5,  73.10   }, //2026.50
//  { 2461314.5,  73.17   }, //2026.75
//  { 2461406.5,  73.32   }, //2027.00
//  { 2461497.5,  73.46   }, //2027.25
//  { 2461588.5,  73.58   }, //2027.50
//  { 2461680.5,  73.66   }  //2027.75

//Note as currently coded there is a single discontinuity of c. 2.5 seconds in October 2027. At this point http://maia.usno.navy.mil/ser7/deltat.preds indicates an error value for DeltaT of about 2 seconds anyway.
};

struct LeapSecondCoefficient
{
  double JD;
  double LeapSeconds;
  double BaseMJD;
  double Coefficient;
};

const LeapSecondCoefficient g_LeapSecondCoefficients[] = //Cumulative leap second values from 1 Jan 1961 to 1 January 2017 as taken from http://maia.usno.navy.mil/ser7/tai-utc.dat
{
  { 2437300.5, 1.4228180, 37300, 0.001296  },
  { 2437512.5, 1.3728180, 37300, 0.001296  },
  { 2437665.5, 1.8458580, 37665, 0.0011232 },
  { 2438334.5, 1.9458580, 37665, 0.0011232 },
  { 2438395.5, 3.2401300, 38761, 0.001296  },
  { 2438486.5, 3.3401300, 38761, 0.001296  },
  { 2438639.5, 3.4401300, 38761, 0.001296  },
  { 2438761.5, 3.5401300, 38761, 0.001296  },
  { 2438820.5, 3.6401300, 38761, 0.001296  },
  { 2438942.5, 3.7401300, 38761, 0.001296  },
  { 2439004.5, 3.8401300, 38761, 0.001296  },
  { 2439126.5, 4.3131700, 39126, 0.002592  },
  { 2439887.5, 4.2131700, 39126, 0.002592  },
  { 2441317.5, 10.0,      41317, 0.0       },
  { 2441499.5, 11.0,      41317, 0.0       },
  { 2441683.5, 12.0,      41317, 0.0       },
  { 2442048.5, 13.0,      41317, 0.0       },
  { 2442413.5, 14.0,      41317, 0.0       },
  { 2442778.5, 15.0,      41317, 0.0       },
  { 2443144.5, 16.0,      41317, 0.0       },
  { 2443509.5, 17.0,      41317, 0.0       },
  { 2443874.5, 18.0,      41317, 0.0       },
  { 2444239.5, 19.0,      41317, 0.0       },
  { 2444786.5, 20.0,      41317, 0.0       },
  { 2445151.5, 21.0,      41317, 0.0       },
  { 2445516.5, 22.0,      41317, 0.0       },
  { 2446247.5, 23.0,      41317, 0.0       },
  { 2447161.5, 24.0,      41317, 0.0       },
  { 2447892.5, 25.0,      41317, 0.0       },
  { 2448257.5, 26.0,      41317, 0.0       },
  { 2448804.5, 27.0,      41317, 0.0       },
  { 2449169.5, 28.0,      41317, 0.0       },
  { 2449534.5, 29.0,      41317, 0.0       },
  { 2450083.5, 30.0,      41317, 0.0       },
  { 2450630.5, 31.0,      41317, 0.0       },
  { 2451179.5, 32.0,      41317, 0.0       },
  { 2453736.5, 33.0,      41317, 0.0       },
  { 2454832.5, 34.0,      41317, 0.0       },
  { 2456109.5, 35.0,      41317, 0.0       },
  { 2457204.5, 36.0,      41317, 0.0       },
  { 2457754.5, 37.0,      41317, 0.0       }
};


////////////////////////////////// Implementation /////////////////////////////

double CAADynamicalTime::DeltaT(double JD) noexcept
{
  //What will be the return value from the method
  double Delta = 0;

  //Determine if we can use the lookup table
  constexpr const size_t nLookupElements = sizeof(g_DeltaTValues) / sizeof(DeltaTValue);
  if ((JD >= g_DeltaTValues[0].JD) && (JD < g_DeltaTValues[nLookupElements - 1].JD))
  {
    //Find the index in the lookup table which contains the JD value closest to the JD input parameter
    bool bFound = false;
    size_t nFoundIndex = 0;
    while (!bFound)
    {
      assert(nFoundIndex < nLookupElements);
      bFound = (g_DeltaTValues[nFoundIndex].JD > JD);

      //Prepare for the next loop
      if (!bFound)
        ++nFoundIndex;
      else
      {
        //Now do a simple linear interpolation of the DeltaT values from the lookup table
        Delta = (JD - g_DeltaTValues[nFoundIndex - 1].JD) / (g_DeltaTValues[nFoundIndex].JD - g_DeltaTValues[nFoundIndex - 1].JD) * (g_DeltaTValues[nFoundIndex].DeltaT - g_DeltaTValues[nFoundIndex - 1].DeltaT) + g_DeltaTValues[nFoundIndex - 1].DeltaT;
      }
    }
  }
  else
  {
    const CAADate date(JD, CAADate::AfterPapalReform(JD));
    const double y = date.FractionalYear();
  
    //Use the polynomial expressions from Espenak & Meeus 2006. References: http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html and
    //http://www.staff.science.uu.nl/~gent0113/deltat/deltat_old.htm (Espenak & Meeus 2006 section)
    if (y < -500)
    {
      const double u = (y - 1820)/100.0;
      const double u2 = u*u;
      Delta = -20 + (32*u2);
    }
    else if (y < 500)
    {
      const double u = y/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      const double u5 = u4*u;
      const double u6 = u5*u;
      Delta = 10583.6 + (-1014.41*u) + (33.78311*u2) + (-5.952053*u3) + (-0.1798452*u4) + (0.022174192*u5) + (0.0090316521*u6);
    }
    else if (y < 1600)
    {
      const double u = (y - 1000)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      const double u5 = u4*u;
      const double u6 = u5*u;
      Delta = 1574.2 + (-556.01*u) + (71.23472*u2) + (0.319781*u3) + (-0.8503463*u4) + (-0.005050998*u5) + (0.0083572073*u6);
    }
    else if (y < 1700)
    {
      const double u = (y - 1600)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      Delta = 120 + (-98.08*u) + (-153.2*u2) + (u3/0.007129);
    }
    else if (y < 1800)
    {
      const double u = (y - 1700)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      Delta = 8.83 + (16.03*u) + (-59.285*u2) + (133.36*u3) + (-u4/0.01174);
    }
    else if (y < 1860)
    {
      const double u = (y - 1800)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      const double u5 = u4*u;
      const double u6 = u5*u;
      const double u7 = u6*u;
      Delta = 13.72 + (-33.2447*u) + (68.612*u2) + (4111.6*u3) + (-37436*u4) + (121272*u5) + (-169900*u6) + (87500*u7);
    }
    else if (y < 1900)
    {
      const double u = (y - 1860)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      const double u5 = u4*u;
      Delta = 7.62 + (57.37*u) + (-2517.54*u2) + (16806.68*u3) + (-44736.24*u4) + (u5/0.0000233174);
    }
    else if (y < 1920)
    {
      const double u = (y - 1900)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      Delta = -2.79 + (149.4119*u) + (-598.939*u2) + (6196.6*u3) + (-19700*u4);
    }
    else if (y < 1941)
    {
      const double u = (y - 1920)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      Delta = 21.20 + (84.493*u) + (-761.00*u2) + (2093.6*u3);
    }
    else if (y < 1961)
    {
      const double u = (y - 1950)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      Delta = 29.07 + (40.7*u) + (-u2/0.0233) + (u3/0.002547);
    }
    else if (y < 1986)
    {
      const double u = (y - 1975)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      Delta = 45.45 + 106.7*u - u2/0.026 - u3/0.000718;
    }
    else if (y < 2005)
    {
      const double u = (y - 2000)/100.0;
      const double u2 = u*u;
      const double u3 = u2*u;
      const double u4 = u3*u;
      const double u5 = u4*u;
      Delta = 63.86 + (33.45*u) + (-603.74*u2) + (1727.5*u3) + (65181.4*u4) + (237359.9*u5);
    }
    else if (y < 2050)
    {
      const double u = (y - 2000)/100.0;
      const double u2 = u*u;
      Delta = 62.92 + (32.217*u) + (55.89*u2);
    }
    else if (y < 2150)
    {
      const double u = (y - 1820)/100.0;
      const double u2 = u*u;
      Delta = -205.72 + (56.28*u) + (32*u2);
    }
    else
    {
      const double u = (y - 1820)/100.0;
      const double u2 = u*u;
      Delta = -20 + (32*u2);
    }
  }

  return Delta;
}

double CAADynamicalTime::CumulativeLeapSeconds(double JD) noexcept
{
  //What will be the return value from the method
  double LeapSeconds = 0;

  constexpr const size_t nLookupElements = sizeof(g_LeapSecondCoefficients) / sizeof(LeapSecondCoefficient);
  if (JD >= g_LeapSecondCoefficients[0].JD)
  {
    //Find the index in the lookup table which contains the JD value closest to the JD input parameter
    bool bContinue = true;
    size_t nIndex = 1;
    while (bContinue)
    {
      if (nIndex >= nLookupElements)
      {
        LeapSeconds = g_LeapSecondCoefficients[nLookupElements - 1].LeapSeconds + (JD - 2400000.5 - g_LeapSecondCoefficients[nLookupElements - 1].BaseMJD) * g_LeapSecondCoefficients[nLookupElements - 1].Coefficient;
        bContinue = false;
      }
      else if (JD < g_LeapSecondCoefficients[nIndex].JD)
      {
        LeapSeconds = g_LeapSecondCoefficients[nIndex - 1].LeapSeconds + (JD - 2400000.5 - g_LeapSecondCoefficients[nIndex - 1].BaseMJD) * g_LeapSecondCoefficients[nIndex - 1].Coefficient;
        bContinue = false;
      }

      //Prepare for the next loop
      if (bContinue)
        ++nIndex;
    }
  }

  return LeapSeconds;
}

double CAADynamicalTime::TT2UTC(double JD) noexcept
{
  //Outside of the range 1 January 1961 to 500 days after the last leap second,
  //we implement TT2UTC as TT2UT1
  constexpr const size_t nLookupElements = sizeof(g_LeapSecondCoefficients) / sizeof(LeapSecondCoefficient);
  if ((JD < g_LeapSecondCoefficients[0].JD) || (JD > (g_LeapSecondCoefficients[nLookupElements - 1].JD + 500)))
    return TT2UT1(JD);

  const double DT = DeltaT(JD);
  const double UT1 = JD - (DT / 86400.0);
  const double LeapSeconds = CumulativeLeapSeconds(JD);
  return ((DT - LeapSeconds - 32.184) / 86400.0) + UT1;
}

double CAADynamicalTime::UTC2TT(double JD) noexcept
{
  //Outside of the range 1 January 1961 to 500 days after the last leap second,
  //we implement TT2UTC as TT2UT1
  constexpr const size_t nLookupElements = sizeof(g_LeapSecondCoefficients) / sizeof(LeapSecondCoefficient);
  if ((JD < g_LeapSecondCoefficients[0].JD) || (JD >(g_LeapSecondCoefficients[nLookupElements - 1].JD + 500)))
    return UT12TT(JD);

  const double DT = DeltaT(JD);
  const double LeapSeconds = CumulativeLeapSeconds(JD);
  const double UT1 = JD - ((DT - LeapSeconds - 32.184) / 86400.0);
  return UT1 + (DT / 86400.0);
}

double CAADynamicalTime::TT2UT1(double JD) noexcept
{
  return JD - (DeltaT(JD) / 86400.0);
}

double CAADynamicalTime::UT12TT(double JD) noexcept
{
  return JD + (DeltaT(JD) / 86400.0);
}

double CAADynamicalTime::UT1MinusUTC(double JD) noexcept
{
  const double JDUTC = JD + ((CAADynamicalTime::DeltaT(JD) - CumulativeLeapSeconds(JD) - 32.184) / 86400);
  return (JD - JDUTC) * 86400;
}
