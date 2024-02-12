/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMDOCUMENT_H_
#define _OTMDOCUMENT_H_

#include "EQF.H"
#include "OTMAPI.H"
#include "EQFTAG.H"
#include "EQFTP.H"
#include "EQFTPI.H"
#include "EQFHLOG.H"

class OtmSegment;
class OtmDocumentProperties;

/*! \brief Abstract base-class for document objects */
class  OtmDocument
{

public:

/*! \brief Constructor */
	OtmDocument() {};

/*! \brief Destructor */
	virtual ~OtmDocument() {};

/*! \brief Read a document.
	Reads the associated files.
	\returns true = success, false = one or more files could not be read. */
	virtual bool read() = 0;

/*! \brief Save a document.
	This writes all associated files (incl. properties) to disk.
	\returns true = success, false = failed to save the document. */
	virtual bool save() = 0;

/*! \brief Close a document.
	This closes all associated files (incl. properties). */
	virtual void close() = 0;

/*! \brief Get the document properties.
	\returns Pointer to the document properties. */
	virtual OtmDocumentProperties* getProperties() = 0;

/*! \brief Get the number of segments in the document
	\returns Number of segments. */
	virtual int getNumberOfSegments() = 0;

/*! \brief Get the segment with the given ID
	\returns Pointer to the segment. */
	virtual OtmSegment* getSegment(int iID) = 0;

/*! \brief Add a segment to the end of the list of segments
	\returns true = success, false = failed to add the segment. */
	virtual bool appendSegment(OtmSegment* pSegment) = 0;

/*! \brief Update a segment
	\returns true = success, false = failed to update the segment. */
	virtual bool updateSegment(OtmSegment* pSegment) = 0;


private:

/*! \brief "Usable"-state of the document-object. */
	OtmPlugin::eUsableState usableState;

};

#endif // #ifndef _OTMDOCUMENT_H_
 
