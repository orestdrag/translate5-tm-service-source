
#include "tm.h"
#include "EncodingHelper.h"

std::wstring getFieldWById(OtmProposal& prop, ProposalFilter::FilterField field)
{
    switch(field)
    {      
        case ProposalFilter::SOURCE:
            return prop.szSource;
        case ProposalFilter::TARGET:
            return prop.szTarget;
        case ProposalFilter::CONTEXT:
            return prop.szContext;
        case ProposalFilter::ADDINFO:
            return prop.szAddInfo;
        default:
            T5LOG(T5ERROR) << "Field " << field << "Not supported yet!";
            return L"";        
    }
}

std::string getFieldById(OtmProposal& prop, ProposalFilter::FilterField field){
    switch(field){
        case ProposalFilter::SRC_LANG:
            return prop.szSourceLanguage;
        case ProposalFilter::TRG_LANG:
            return prop.szTargetLanguage;

        case ProposalFilter::DOC:
            return prop.szDocName;
        case ProposalFilter::AUTHOR:
            return prop.szTargetAuthor;

        default:
            T5LOG(T5ERROR) << "Field " << field << "Not supported yet!";
            return "";        
    }
}

std::wstring convertStrToWstr(std::string& str)
{
    return EncodingHelper::convertToWChar(str);
}

std::string convertWstrToStr(std::wstring& wstr)
{
    return EncodingHelper::convertToUTF8(wstr);
}



const std::map<ProposalFilter::FilterField, std::string> FilterFieldToStrMap{
    {ProposalFilter::UNKNOWN_FIELD,"UNKNOWN_FIELD"},
    {ProposalFilter::TIMESTAMP,"TIMESTAMP"},
    {ProposalFilter::SRC_LANG,"SRC_LANG"},
    {ProposalFilter::TRG_LANG,"TRG_LANG"},
    {ProposalFilter::AUTHOR,"AUTHOR"},
    {ProposalFilter::DOC,"DOCUMENT"},
    {ProposalFilter::SOURCE,"SOURCE"},
    {ProposalFilter::TARGET,"TARGET"},
    {ProposalFilter::CONTEXT,"CONTEXT"},
    {ProposalFilter::ADDINFO,"ADDINFO"},
};

std::string ProposalFilter::toString()const
{
    auto filterField = FilterFieldToStrMap.find(m_field);
    std::string result = "Search filter, field: " + (filterField!=FilterFieldToStrMap.end()? filterField->second : "Not found");
    // add field type 
    //if timestamp 

    //else
    // add filter type
    if(m_type == ProposalFilter::EXACT)
    {
        result += " FilterType::EXACT";
    }else if(m_type == ProposalFilter::CONTAINS)
    {
        result += " FilterType::CONTAINS";
    }else if(m_type == ProposalFilter::NONE)
    {
        result += " FilterType::NONE";
    }else if(m_type == ProposalFilter::RANGE)
    {
        result += " FilterType::RANGE";
    }else 
    {
        result += " FilterType::UNKNOWN";
    }

    //add search string
    if(m_field == ProposalFilter::TIMESTAMP)
    {
        char szDateTime[100];
        convertTimeToUTC( m_timestamp1, szDateTime );
        std::string time1 = szDateTime;
        szDateTime[0] = '\0';

        convertTimeToUTC( m_timestamp2, szDateTime );
        std::string time2 = szDateTime;
        result += " Range: " + time1 + " - " + time2 ;
    }
    else
    {  
        result += " SearchStr: '" + m_searchString + "';";
    }

    //add options
    result += " Options: ";
    if ( m_searchOptions & SEARCH_FILTERS_NOT )
    {
        result += "SEARCH_FILTERS_NOT|";
    }
    if ( m_searchOptions & SEARCH_CASEINSENSITIVE_OPT )
    {
        result += "SEARCH_CASEINSENSITIVE_OPT|";
    }
    if ( m_searchOptions & SEARCH_WHITESPACETOLERANT_OPT )
    {
        result += "SEARCH_WHITESPACETOLERANT_OPT|";
    }
    return result;
}

bool ProposalFilter::check(OtmProposal& prop)
{
    bool result = false;

    if(m_field == ProposalFilter::TIMESTAMP)
    {
        result = (prop.lTargetTime >= m_timestamp1 && prop.lTargetTime <= m_timestamp2);
    }
    else if(m_field >= ProposalFilter::WIDE_CHAR_FIELDS_START)
    {
        std::wstring searchWStr = getFieldWById(prop, m_field);
        if ( m_searchOptions & SEARCH_CASEINSENSITIVE_OPT )
        {
            wcsupr( &searchWStr[0] );
        }
        if ( m_searchOptions & SEARCH_WHITESPACETOLERANT_OPT )
        {
            normalizeWhiteSpace( &searchWStr[0] );
        }

        if(m_type == ProposalFilter::EXACT)
        {
            result = (searchWStr == m_searchStringW);
        }else if(m_type == ProposalFilter::CONTAINS)
        {
            result = (searchWStr.find(m_searchStringW) != std::string::npos);
        }
    }
    else
    {
        std::string searchStr = getFieldById(prop, m_field);
        if ( m_searchOptions & SEARCH_CASEINSENSITIVE_OPT )
        {
            strupr( &searchStr[0] );
        }
        if ( m_searchOptions & SEARCH_WHITESPACETOLERANT_OPT )
        {
            std::wstring wSearchStr = convertStrToWstr(searchStr);
            normalizeWhiteSpace( &wSearchStr[0] );
            searchStr = convertWstrToStr(wSearchStr);
        }

        if(m_type == ProposalFilter::EXACT)
        {
            result =  (searchStr == m_searchString);
        }else if(m_type == ProposalFilter::CONTAINS)
        {
            result = (searchStr.find(m_searchString) != std::string::npos);
        }
    }

    //if filter is reverted we shoud return oposite value
    if(m_searchOptions & SEARCH_FILTERS_NOT) 
        return !result;
    return result;
}
