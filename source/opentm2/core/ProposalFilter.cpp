
#include "tm.h"
#include "EncodingHelper.h"

std::wstring getFieldWById(OtmProposal& prop, ProposalFilter::FilterField field){
    switch(field){      
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
//if(type >= WIDE_CHAR_FIELDS_START){
 //   m_searchStringW = EncodingHelper::convertToUTF32(search);
//}
//    m_searchString = search;
    return EncodingHelper::convertToWChar(str);
}

/*
bool ProposalFilter::check(OtmProposal& prop){
    if(m_field == ProposalFilter::TIMESTAMP){
        return prop.lTargetTime >= m_timestamp1 && prop.lTargetTime <= m_timestamp2;
    }

    if(m_field >= ProposalFilter::WIDE_CHAR_FIELDS_START){
        std::wstring searchWStr = getFieldWById(prop, m_field);
        if(m_type == ProposalFilter::EXACT){
            return searchWStr == m_searchStringW;
        }else if(m_type == ProposalFilter::CONTAINS){
            return searchWStr.find(m_searchStringW) != std::string::npos;
        }
    }else{
        std::string searchStr = getFieldById(prop, m_field);
        
        if(m_type == ProposalFilter::EXACT){
            return searchStr == m_searchString;
        }else if(m_type == ProposalFilter::CONTAINS){
            return searchStr.find(m_searchString) != std::string::npos;
        }
    }

    return false;
}
//*/

bool ProposalFilter::check(OtmProposal& prop){
    if(m_field == ProposalFilter::TIMESTAMP){
        return prop.lTargetTime >= m_timestamp1 && prop.lTargetTime <= m_timestamp2;
    }

    if(m_field >= ProposalFilter::WIDE_CHAR_FIELDS_START){
        std::wstring searchWStr = getFieldWById(prop, m_field);
        if(m_type == ProposalFilter::EXACT){
            return searchWStr == m_searchStringW;
        }else if(m_type == ProposalFilter::CONTAINS){
            return searchWStr.find(m_searchStringW) != std::string::npos;
        }
    }else{
        std::string searchStr = getFieldById(prop, m_field);
        
        if(m_type == ProposalFilter::EXACT){
            return searchStr == m_searchString;
        }else if(m_type == ProposalFilter::CONTAINS){
            return searchStr.find(m_searchString) != std::string::npos;
        }
    }

    return false;
}
