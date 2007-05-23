/****************************************************************************
*   File:       XMLUtils.cpp
*   Author:     S. Skorokhodov
*
*   Description:
*       Singleton wrapper class for Xerces-C/Xalan-C for biosig4c++ library.       
*****************************************************************************
*/
#include <iostream>
#include <cassert>
#include <memory>
#include <string>
//---------------------------------------------------------------------------
#include <boost/smart_ptr.hpp>
//---------------------------------------------------------------------------
//  Xerces includes
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
//  XQilla includes
#include <xqilla/xqilla-dom3.hpp>
//---------------------------------------------------------------------------
#include "XMLUtils.h"
//---------------------------------------------------------------------------
XERCES_CPP_NAMESPACE_USE;
//---------------------------------------------------------------------------
namespace BioSig
{
//===========================================================================
//  Private methods of class aECGXMLUtils
//===========================================================================
void aECGXMLUtils::init_()
{
    XQillaPlatformUtils::initialize();
    pdomimpl_   =   DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
    if ( !pdomimpl_ ) throw aECGXMLUtils::xml_err();
    pparser_    =   pdomimpl_->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    pparser_->setFeature(X("namespaces"), true);
    pparser_->setFeature(X("http://apache.org/xml/features/validation/schema"), false);
    pparser_->setFeature(X("validation"), false);
}
//---------------------------------------------------------------------------
void aECGXMLUtils::clean_up_doc_()
{
    if ( presolver_ )
    {
        presolver_->release();
        presolver_  =   0;
    }
    if ( pdoc_ )
    {
        pdoc_->release();
        pdoc_       =   0;
    }
}
//---------------------------------------------------------------------------
void aECGXMLUtils::do_parse_uri_( const char *uri )
{
    assert( pparser_ );
    pdoc_           =   pparser_->parseURI( uri );
    if ( pdoc_ )
    {
        presolver_  =   (XQillaNSResolver*)pdoc_->createNSResolver(pdoc_->getDocumentElement());
        //  add prefix for default namespace
        presolver_->addNamespaceBinding(X("hl7"), X("urn:hl7-org:v3"));
    }
}
//---------------------------------------------------------------------------
XPath2Result *aECGXMLUtils::do_select_node_iterator_( const char *pexpr, DOMNode *pcontext )
{
    assert( pexpr );
    assert( pdoc_ );
    assert( presolver_ );
    DOMNode *actual_context =   pcontext ? pcontext : pdoc_->getDocumentElement();
    const DOMXPathExpression*
        parsed_expr         =   pdoc_->createExpression( X(pexpr), presolver_ );
    return (XPath2Result*)parsed_expr->evaluate(    actual_context,
                                                    XPath2Result::ITERATOR_RESULT,
                                                    0   );
}
//===========================================================================
//  class aECGXMLUtils public interface
//===========================================================================
aECGXMLUtils::aECGXMLUtils() : pdomimpl_(0), pparser_(0), pdoc_(0), presolver_(0)
{
    try
    {
        init_();
    }
    catch ( const XMLException& e )
    {
        (void)e;
        throw xml_err();
    }
    catch ( const DOMException &e )
    {
        (void)e;
        throw dom_err();
    }
    catch ( const DOMXPathException &e )
    {
        (void)e;
        throw xpath_err();
    }
    catch ( ... )
    {
        throw unexp_err();
    }
}
//---------------------------------------------------------------------------
aECGXMLUtils::~aECGXMLUtils()
{
     XQillaPlatformUtils::terminate();;
}
//---------------------------------------------------------------------------
XERCES_CPP_NAMESPACE::DOMBuilder *aECGXMLUtils::getCompatableParser()
{
    return pparser_;
}
//---------------------------------------------------------------------------
void aECGXMLUtils::parseURI( const char *uri )
{
    try
    {
        clean_up_doc_();
        do_parse_uri_( uri );
    }
    catch ( const XMLException& e )
    {
        (void)e;
        clean_up_doc_();
        throw xml_err();
    }
    catch ( const DOMException &e )
    {
        (void)e;
        clean_up_doc_();
        throw dom_err();
    }
    catch ( const DOMXPathException &e )
    {
        (void)e;
        clean_up_doc_();
        throw xpath_err();
    }
    catch ( ... )
    {
        clean_up_doc_();
        throw unexp_err();
    }
}
//---------------------------------------------------------------------------
void aECGXMLUtils::detachDoc()
{
    clean_up_doc_();
}
//---------------------------------------------------------------------------
DOMDocument *aECGXMLUtils::getParsedDoc( const char *uri )
{
    if ( !(pdoc_ && presolver_) )
        throw no_doc();
    return pdoc_;
}
//---------------------------------------------------------------------------
XQillaNSResolver *aECGXMLUtils::getCompatableNSResolver()
{
    if ( !(pdoc_ && presolver_) )
        throw no_doc();
    return presolver_;
}
//---------------------------------------------------------------------------
DOMImplementation *aECGXMLUtils::getCompatableImplementation()
{
    assert( pdomimpl_ );
    return pdomimpl_;
}
//---------------------------------------------------------------------------
XPath2Result *aECGXMLUtils::selectNodeSet( const char *pexpr, DOMNode *pcontext )
{
    if ( !(pdoc_ && presolver_) )
        throw no_doc();

    XPath2Result *pres  =   0;
    try
    {
        pres            =   do_select_node_iterator_( pexpr, pcontext );
    }
    catch ( const XMLException& e )
    {
        (void)e;
    }
    catch ( const DOMException &e )
    {
        (void)e;
    }
    catch ( const DOMXPathException &e )
    {
        (void)e;
    }
    catch ( ... )
    {
    }
    return pres;
}
//===========================================================================
//  aECGXML singleton
//===========================================================================
aECGXMLUtils *aECGXML::instance()
{
    static boost::scoped_ptr<aECGXMLUtils> apinstance(0);
    if ( apinstance.get() == NULL )
    {
        apinstance.reset( new aECGXMLUtils );
    }
    return apinstance.get();
}
//---------------------------------------------------------------------------
}   //  namespace BioSig
//---------------------------------------------------------------------------
