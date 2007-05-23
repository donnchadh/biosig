/****************************************************************************
*   File:       sreader.cpp
*   Author:     S. Skorokhodov
*
*   Description:
*       Simple reader for contents of HL7 v3 AnnotatedECG files.
*****************************************************************************
*/
#include <iostream>
#include <cassert>
#include <memory>
#include <string>
//---------------------------------------------------------------------------
//  Xerces includes
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
//  XQilla includes
#include <xqilla/xqilla-dom3.hpp>
//---------------------------------------------------------------------------
#include "XMLUtils.h"
#include "aecg_parser.h"
//---------------------------------------------------------------------------
XERCES_CPP_NAMESPACE_USE;
using namespace std;
//===========================================================================
//  Constants
//===========================================================================
const char*   SUBJ_ASSING_ID_QRY                =   "/hl7:AnnotatedECG"
                                                    "/hl7:componentOf"
                                                    "/hl7:timepointEvent"
                                                    "/hl7:componentOf"
                                                    "/hl7:subjectAssignment"
                                                    "/hl7:subject"
                                                    "/hl7:trialSubject"
                                                    "/hl7:id";
const char*   SUBJ_ASSING_ID_QRY1               =   "/AnnotatedECG"
                                                    "/componentOf"
                                                    "/timepointEvent"
                                                    "/componentOf"
                                                    "/subjectAssignment"
                                                    "/subject"
                                                    "/trialSubject"
                                                    "/id";
const char*   TRIAL_SUBJ_QRY                    =   "/hl7:AnnotatedECG"
                                                    "/hl7:componentOf"
                                                    "/hl7:timepointEvent"
                                                    "/hl7:componentOf"
                                                    "/hl7:subjectAssignment"
                                                    "/hl7:subject"
                                                    "/hl7:trialSubject"
                                                    "/hl7:id"
                                                    "/@extension";
const char*   TRIAL_SUBJ_QRY1                   =   "/AnnotatedECG"
                                                    "/componentOf"
                                                    "/timepointEvent"
                                                    "/componentOf"
                                                    "/subjectAssignment"
                                                    "/subject"
                                                    "/trialSubject";
//===========================================================================
//  Main
//===========================================================================
int main( int	argc, char*	argv[] )
{
  
    using namespace BioSig;
    try 
    {
        BioSig::Simple_aECGReader sr( "./aecg.xml" );
        cout << "Patient ID: " << sr.subj_id() << endl;
        cout << "Patient name: " << sr.subj_name() << endl;
        switch ( sr.subj_sex() )
        {
        case 1:
            cout << "Patient is a male" << endl;
            break;

        case 2:
            cout << "Patient is a lady" << endl;
            break;

        default:
            cout << "Patient sex is unknown" << endl;
            break;
        }
    }
    catch ( const aECGXMLUtils::xml_err &e )
    {
        (void)e;
        cout    <<  "XML initializing error"    <<  endl;
        return EXIT_FAILURE;
    }
    catch ( const aECGXMLUtils::dom_err &e )
    {
        (void)e;
        cout    <<  "DOM initializing error"    <<  endl;
        return EXIT_FAILURE;
    }
    catch ( const aECGXMLUtils::xpath_err &e )
    {
        (void)e;
        cout    <<  "XPath initializing error"  <<  endl;
        return EXIT_FAILURE;
    }
    catch ( const aECGXMLUtils::no_doc &e )
    {
        (void)e;
        cout    <<  "No document in operation that requires it" <<  endl;
    }
    catch ( const aECGXMLUtils::unexp_err &e )
    {
        (void)e;
        cout    <<  "Unexpected initializing exception" <<  endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
