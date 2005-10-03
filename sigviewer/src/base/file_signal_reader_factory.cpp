// file_signal_reader_factory.h

#include "file_signal_reader_factory.h"

// all suported readers
#include "gdf/gdf_reader.h"

namespace BioSig_
{

// instance
std::auto_ptr<FileSignalReaderFactory> FileSignalReaderFactory::instance_;

// get instance
FileSignalReaderFactory* FileSignalReaderFactory::getInstance()
{
    if (!instance_.get())
    {
        instance_.reset(new FileSignalReaderFactory);

        // register all readers
        instance_->addPrototype(".evt", new GDFReader);
        instance_->addPrototype(".gdf", new GDFReader);
    }
    return instance_.get();
}

// get extensions
QString FileSignalReaderFactory::getExtensions()
{
    QString extensions;
    for (StringIterator iter = getElementNameBegin();
         iter != getElementNameEnd();
         iter++)
    {
        if ((*iter)[0] == '.')
        {
            extensions += "*" + (*iter) + " ";
        }
    }
    if (extensions.length() > 0)
    {
        extensions = extensions.mid(0, extensions.length() - 1);
    }
    return extensions;
}

} // namespace BioSig_
