/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// SQLLite functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef HAVE_SQLITE

//---------------------------------------------------------------------------
#ifndef SQLLiteReportH
#define SQLLiteReportH
//---------------------------------------------------------------------------

#include "DatabaseReport.h"
#include "SQLLite.h"
#include <sqlite3.h>
//---------------------------------------------------------------------------

namespace MediaConch {

//***************************************************************************
// Class SQLLiteReport
//***************************************************************************

class SQLLiteReport : public DatabaseReport, public SQLLite
{
public:
    //Constructor/Destructor
    SQLLiteReport();
    virtual ~SQLLiteReport();

    //Init
    virtual int create_report_table();
    virtual int update_report_table();

    // File
    virtual long add_file(const std::string& filename, const std::string& file_last_modification,
                          std::string& err,
                          long source_id=-1,
                          const std::string& generated_file="", const std::string& log="");
    virtual long get_file_id(const std::string& file, const std::string& file_last_modification);
    virtual void get_file_from_id(long id, std::string& file);

    // Report
    virtual int save_report(long file_id, MediaConchLib::report reportKind, MediaConchLib::format format,
                            const std::string& report, MediaConchLib::compression,
                            int mil_version);
    virtual int update_report(long file_id, MediaConchLib::report reportKind, MediaConchLib::format format,
                              const std::string& report, MediaConchLib::compression,
                              int mil_version);
    virtual int remove_report(long filename);
    virtual void get_report(long file_id, MediaConchLib::report reportKind, MediaConchLib::format format,
                            std::string& report, MediaConchLib::compression&);
    virtual bool report_is_registered(long file_id, MediaConchLib::report reportKind, MediaConchLib::format format);
    virtual int  version_registered(long file_id);
    virtual void get_elements(std::vector<std::string>& vec);
    virtual void get_element_report_kind(long file_id, MediaConchLib::report& report_kind);

protected:
    virtual int init();
    virtual int init_report();

private:
    int           report_version;
    static int    current_report_version;

    SQLLiteReport (const SQLLiteReport&);
    SQLLiteReport& operator=(const SQLLiteReport&);
};

}

#endif /* !SQLLiteReportH */

#endif /* !HAVE_SQLITE */
