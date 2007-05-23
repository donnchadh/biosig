/****************************************************************************
*   File:           aecg_parser.cpp
*   Copyright:      S. Skorokhodov, 2006
*
*   Description:
*       A class for reading HL7 v3 Annotated ECG file and exposing its
*       contents in the form apropriate for BioSig format.
*       Part of biosig4c++ application.
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
#include <xercesc/util/XMLString.hpp>
//---------------------------------------------------------------------------
#include "aecg_parser.h"
#include "XMLUtils.h"
//---------------------------------------------------------------------------
XERCES_CPP_NAMESPACE_USE;
//===========================================================================
//  XPath expressions
//===========================================================================
namespace
{
//---------------------------------------------------------------------------
const char *PATIENT_ROOT_QRY        =   "/hl7:AnnotatedECG"
                                        "/hl7:componentOf"
                                        "/hl7:timepointEvent"
                                        "/hl7:componentOf"
                                        "/hl7:subjectAssignment"
                                        "/hl7:subject"
                                        "/hl7:trialSubject";
const char *PATIENT_ID_QRY          =   "/hl7:id"
                                        "/@extension";
const char *PATIENT_NAME_QRY        =   "/hl7:subjectDemographicPerson"
                                        "/hl7:name";
const char *PAT_GENERIC_NM_SQRY     =   "./text()";
const int   BIOSIG_SEX_UNKNOWN      =   0;
const int   BIOSIG_SEX_MALE         =   1;
const int   BIOSIG_SEX_FEMALE       =   2;
const char *PATIENT_SEX_QRY         =   "/hl7:subjectDemographicPerson"
                                        "/hl7:administrativeGenderCode"
                                        "/@code";
//---------------------------------------------------------------------------
}   //  anonimous namespace
//---------------------------------------------------------------------------
namespace BioSig
{
//===========================================================================
//  Internal implementation of Simple_aECGReader
//===========================================================================
struct Simple_aECGReader::Simple_aECGReaderImpl
{
    //-----------------------------------------------------------------------
    Simple_aECGReaderImpl()
    :   pxml(0),
        patient_id(""), patient_name(""), patient_sex(BIOSIG_SEX_UNKNOWN)
    {
        attach_to_xml();
    }
    //-----------------------------------------------------------------------
    ~Simple_aECGReaderImpl()
    {
        if ( pxml )
        {
            pxml->detachDoc();
        }
    }
    //-----------------------------------------------------------------------
    bool is_valid()
    {
        bool ret        =   true;
        if ( !pxml )
            ret         =   false;
        return ret;
    }
    //-----------------------------------------------------------------------
    void attach_to_xml()
    {
        pxml            =   aECGXML::instance();
    }
    //-----------------------------------------------------------------------
    void parse_aecg_file( const char *uri )
    {
        assert( pxml );
        pxml->parseURI( uri );
    }
    //-----------------------------------------------------------------------
    void retrieve_patient_info();
    //-----------------------------------------------------------------------
    void retrieve_patient_id();
    //-----------------------------------------------------------------------
    void retrieve_patient_name();
    //-----------------------------------------------------------------------
    void retrieve_patient_sex();
    //=======================================================================
    //  Data members
    //=======================================================================
    aECGXMLUtils       *pxml;
    std::string         patient_id;
    std::string         patient_name;
    int                 patient_sex;
};  //  struct Simple_aECGReader::Simple_aECGReaderImpl
//---------------------------------------------------------------------------
void Simple_aECGReader::Simple_aECGReaderImpl::retrieve_patient_info()
{
    retrieve_patient_id();
    retrieve_patient_name();
    retrieve_patient_sex();
}
//---------------------------------------------------------------------------
void Simple_aECGReader::Simple_aECGReaderImpl::retrieve_patient_id()
{
    std::string qry         =   PATIENT_ROOT_QRY;
    qry                    +=   PATIENT_ID_QRY;
    assert( pxml );
    XPath2Result *pres      =   pxml->selectNodeSet( qry.c_str(), 0 );
    assert( pres );
    if ( pres->iterateNext() )
    {
        const DOMNode *pn   =   pres->asNode();
        const XMLCh   *pxc  =   pn->getNodeValue();
        char          *pch  =   XMLString::transcode( pxc );
        if ( pch )
        {
            patient_id      =   pch;
            XMLString::release( &pch );
        }
    }
}
//---------------------------------------------------------------------------
void Simple_aECGReader::Simple_aECGReaderImpl::retrieve_patient_name()
{
    std::string     qry     =   PATIENT_ROOT_QRY;
    qry                    +=   PATIENT_NAME_QRY;
    assert( pxml );
    XPath2Result   *pres    =   pxml->selectNodeSet( qry.c_str(), 0 );
    assert( pres );
    if ( pres->iterateNext() )
    {
        DOMNode *pname          =   const_cast<DOMNode*>(pres->asNode());
        if ( pname )
        {
            DOMNodeList *plst   =   pname->getChildNodes();
            if ( plst )
            {
                XMLSize_t sz    =   plst->getLength();
                std::string prefix(""), given(""), family("");
                for ( XMLSize_t i = 0; i < sz; ++i )
                {
                    DOMNode *ptmp   =   plst->item( i );
                    assert( ptmp );
                    switch ( ptmp->getNodeType() )
                    {
                    case DOMNode::ELEMENT_NODE:
                        //  according to HL7 v3 specs living subject name
                        //  must be represented either as text content of name tag
                        //  (i.e. trival name) or as structured content
                        if ( patient_name.empty() )
                        {
                            const XMLCh* nn =   ptmp->getLocalName();
                            if ( XMLString::equals( nn, X("prefix") ) )
                            {
                                const XMLCh *pxc    =   ptmp->getTextContent();
                                assert( pxc );
                                char        *pch    =   XMLString::transcode( pxc );
                                assert( pch );
                                prefix              =   pch;
                                XMLString::release( &pch );

                            }
                            else if ( XMLString::equals( nn, X("given") ) )
                            {
                                const XMLCh *pxc    =   ptmp->getTextContent();
                                assert( pxc );
                                char        *pch    =   XMLString::transcode( pxc );
                                assert( pch );
                                given               =   pch;
                                XMLString::release( &pch );
                            }
                            else if ( XMLString::equals( nn, X("family") ) )
                            {
                                const XMLCh *pxc    =   ptmp->getTextContent();
                                assert( pxc );
                                char        *pch    =   XMLString::transcode( pxc );
                                assert( pch );
                                family              =   pch;
                                XMLString::release( &pch );
                            }
                        }
                        break;

                    case DOMNode::TEXT_NODE:
                        {
                            const XMLCh *pxcsrc =   ptmp->getNodeValue();
                            assert( pxcsrc );
                            if ( !XMLString::isAllWhiteSpace(pxcsrc) )
                            {
                                XMLCh *pxc      =   XMLString::replicate( pxcsrc );
                                assert( pxc );
                                XMLString::trim( pxc );
                                char *pch       =   XMLString::transcode( pxc );
                                assert( pch );
                                patient_name    =   pch;
                                XMLString::release( &pxc );
                                XMLString::release( &pch );
                            }
                        }
                        break;

                    default:
                        continue;
                    }
                }
                if ( patient_name.empty() )
                {
                    //  no trival name found so we need to compose the patient name
                    assert( !(given.empty() && family.empty()) );
                    patient_name    =   prefix + " " + given + " " + family;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void Simple_aECGReader::Simple_aECGReaderImpl::retrieve_patient_sex()
{
    std::string qry         =   PATIENT_ROOT_QRY;
    qry                    +=   PATIENT_SEX_QRY;
    assert( pxml );
    XPath2Result *pres      =   pxml->selectNodeSet( qry.c_str(), 0 );
    assert( pres );
    XPath2Result::ResultType rt =   pres->getResultType();
    if ( pres->iterateNext() )
    {
        const DOMNode *pn   =   pres->asNode();
        const XMLCh   *pxc  =   pn->getNodeValue();
        if ( XMLString::equals( pxc, X("M") ) )
        {
            patient_sex     =   BIOSIG_SEX_MALE;
        }
        else if ( XMLString::equals( pxc, X("F") ) )
        {
            patient_sex     =   BIOSIG_SEX_FEMALE;
        }
        else
        {
            patient_sex     =   BIOSIG_SEX_UNKNOWN;
        }
    }
}
//===========================================================================
//  class Simple_aECGReader
//===========================================================================
Simple_aECGReader::Simple_aECGReader( const char *pfile_name ) : pimpl_(0)
{
    try
    {
        std::auto_ptr<Simple_aECGReaderImpl> apimpl( new Simple_aECGReaderImpl );
        apimpl->parse_aecg_file( pfile_name );
        apimpl->retrieve_patient_info();
        pimpl_          =   apimpl.release();
    }
    catch ( const aECGXMLUtils::xml_err &e )
    {
        (void)e;
    }
    catch ( const aECGXMLUtils::dom_err &e )
    {
        (void)e;
    }
    catch ( const aECGXMLUtils::xpath_err &e )
    {
        (void)e;
    }
    catch ( const aECGXMLUtils::no_doc &e )
    {
        (void)e;
    }
    catch ( const aECGXMLUtils::unexp_err &e )
    {
        (void)e;
    }
}
//---------------------------------------------------------------------------
Simple_aECGReader::~Simple_aECGReader()
{
    delete pimpl_;
}
//---------------------------------------------------------------------------
const char *Simple_aECGReader::subj_id() const
{
    return pimpl_->patient_id.c_str();
}
//---------------------------------------------------------------------------
const char *Simple_aECGReader::subj_name() const
{
    return pimpl_->patient_name.c_str();
}
//---------------------------------------------------------------------------
int Simple_aECGReader::subj_sex() const
{
    return pimpl_->patient_sex;
}
//---------------------------------------------------------------------------
}   //  namespace BioSig
//---------------------------------------------------------------------------
