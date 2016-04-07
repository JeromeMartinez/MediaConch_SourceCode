/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#include "WebChoose.h"

#if defined(WEB_MACHINE_KIT)

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>
#include <QFileDialog>
#include <QFile>
#include <QTextDocument>
#include <QTextStream>

#include "Common/FileRegistered.h"
#include "mainwindow.h"
#include "WebPage.h"
#include <QtGlobal>
#include <QDebug>

namespace MediaConch
{
    WebPage::WebPage(MainWindow *m, QWidget *parent) : QWebPage(parent), mainwindow(m)
    {
        connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    }

    void WebPage::onLoadFinished(bool ok)
    {
        if (!ok)
            return;

        QWebFrame* frame = mainFrame();
        frame->addToJavaScriptWindowObject("webpage", this);
        connect(this, SIGNAL(update_registered_file(FileRegistered*)),
                this, SLOT(update_status_registered_file(FileRegistered*)));
    }

    void WebPage::menu_link_checker(const QString& name)
    {
        if (!name.compare("Checker"))
            mainwindow->checker_selected();
        else if (!name.compare("Policies"))
            mainwindow->policies_selected();
        else if (!name.compare("Display"))
            mainwindow->display_selected();
        else if (!name.compare("Help"))
            mainwindow->on_actionGettingStarted_triggered();
        else
            mainwindow->checker_selected();
    }

    void WebPage::onInputChanged(const QString& inputName)
    {
        select_file_name = inputName;
    }

    void WebPage::onButtonClicked(const QString& id)
    {
        QWebElement button = mainFrame()->findFirstElement(id);
        if (id == "#file")
            onFileUploadSelected(button);
        else if (id == "#url")
            onFileOnlineSelected(button);
        else if (id == "#repository")
            onFileRepositorySelected(button);
    }

    void WebPage::onFillImplementationReport(const QString& file, const QString& target, const QString& display, const QString& verbosity)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        QString report;
        int display_i = display.toInt();
        int *verbosity_p = NULL;
        int verbosity_i;
        if (verbosity.length())
        {
            verbosity_i = verbosity.toInt();
            verbosity_p = &verbosity_i;
        }

        mainwindow->get_implementation_report(file_s, report, &display_i, verbosity_p);
        QWebElement form = mainFrame()->findFirstElement(target + " .modal-body");

        if (report_is_html(report))
            form.setInnerXml(report);
        else
            form.setPlainText(report);
    }

    void WebPage::onFillPolicyReport(const QString& file, const QString& target, const QString& policy, const QString& display)
    {
        int policy_i = policy.toInt();
        QString report;
        if (policy_i != -1)
        {
            std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
            int display_i = display.toInt();
            mainwindow->validate_policy(file_s, report, policy_i, &display_i);
        }
        QWebElement form = mainFrame()->findFirstElement(target + " .modal-body");

        if (report_is_html(report))
            form.setInnerXml(report);
        else
            form.setPlainText(report);
    }

    QString WebPage::onFillMediaInfoReport(const QString& file)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        return mainwindow->get_mediainfo_jstree(file_s);
    }

    QString WebPage::onFillMediaTraceReport(const QString& file)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        return mainwindow->get_mediatrace_jstree(file_s);
    }

    void WebPage::onDownloadReport(const QString& report, const QString& save_name)
    {
        if (report.isEmpty())
            return;

        QString proposed = save_name;
        bool is_html = report_is_html(report);
        bool is_xml = false;
        if (is_html)
            proposed.replace(proposed.length() - 3, 3, "html");
        else
        {
            is_xml = report_is_xml(report);
            if (is_xml)
                proposed.replace(proposed.length() - 3, 3, "xml");
            else
                proposed.replace(proposed.length() - 3, 3, "txt");
        }

        QString dl_file = QFileDialog::getSaveFileName(view(), "Save report", proposed);

        if (!dl_file.length())
            return;

        QFile file(dl_file);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        out << report;
    }

    void WebPage::onSaveImplementationReport(const QString& file, const QString& save_name, const QString& display, const QString& verbosity)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        QString report;
        int display_i = display.toInt();
        int *verbosity_p = NULL;
        int verbosity_i;
        if (verbosity.length())
        {
            verbosity_i = verbosity.toInt();
            verbosity_p = &verbosity_i;
        }
        mainwindow->get_implementation_report(file_s, report, &display_i, verbosity_p);
        onDownloadReport(report, save_name);
    }

    void WebPage::onSavePolicyReport(const QString& file, const QString& save_name, const QString& policy, const QString& display)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        QString report;
        int policy_i = policy.toInt();
        int display_i = display.toInt();
        mainwindow->validate_policy(file_s, report, policy_i, &display_i);
        onDownloadReport(report, save_name);
    }

    void WebPage::onSaveInfo(const QString& file, const QString& save_name)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        std::string display_name, display_content;
        QString report = mainwindow->get_mediainfo_xml(file_s, display_name, display_content);
        onDownloadReport(report, save_name);
    }

    void WebPage::onSaveTrace(const QString& file, const QString& save_name)
    {
        std::string file_s = std::string(file.toUtf8().data(), file.toUtf8().length());
        std::string display_name, display_content;
        QString report = mainwindow->get_mediatrace_xml(file_s, display_name, display_content);
        onDownloadReport(report, save_name);
    }

    int WebPage::onFileUploadSelected(QWebElement& form)
    {
        QStringList files = file_selector.value("checkerUpload_file", QStringList());
        if (!files.size())
            return 1;

        QWebElement policyElement = form.findFirst("#checkerUpload_policy");
        QString policy = policyElement.evaluateJavaScript("this.value").toString();

        QWebElement displayElement = form.findFirst("#checkerUpload_display_selector");
        QString display_xslt = displayElement.evaluateJavaScript("this.value").toString();

        QWebElement verbosityElement = form.findFirst("#checkerUpload_verbosity_selector");
        QString verbosity = verbosityElement.evaluateJavaScript("this.value").toString();

        for (int i = 0; i < files.size(); ++i)
        {
            QFileInfo f = QFileInfo(files[i]);
            mainwindow->add_file_to_list(f.fileName(), f.absolutePath(), policy, display_xslt, verbosity);
        }

        return 0;
    }

    int WebPage::onFileOnlineSelected(QWebElement& form)
    {
        QWebElement urlElement = form.findFirst("#checkerOnline_file");
        QString url = urlElement.evaluateJavaScript("this.value").toString();

        if (!url.length())
            return 1;

        QWebElement policyElement = form.findFirst("#checkerOnline_policy");
        QString policy = policyElement.evaluateJavaScript("this.value").toString();
        QWebElement displayElement = form.findFirst("#checkerOnline_display_selector");
        QString display_xslt = displayElement.evaluateJavaScript("this.value").toString();
        QWebElement verbosityElement = form.findFirst("#checkerOnline_display_selector");
        QString verbosity = verbosityElement.evaluateJavaScript("this.value").toString();

        mainwindow->add_file_to_list(url, "", policy, display_xslt, verbosity);
        return 0;
    }

    int WebPage::onFileRepositorySelected(QWebElement& form)
    {
        QStringList dirname = file_selector.value("checkerRepository_directory", QStringList());
        if (dirname.empty())
            return 1;

        QDir dir(dirname.last());
        QFileInfoList list = dir.entryInfoList(QDir::Files);
        if (!list.count())
            return 1;

        QWebElement policyElement = form.findFirst("#checkerRepository_policy");
        QString policy = policyElement.evaluateJavaScript("this.value").toString();
        QWebElement displayElement = form.findFirst("#checkerRepository_display_selector");
        QString display_xslt = displayElement.evaluateJavaScript("this.value").toString();
        QWebElement verbosityElement = form.findFirst("#checkerRepository_verbosity_selector");
        QString verbosity = verbosityElement.evaluateJavaScript("this.value").toString();

        for (int i = 0; i < list.size(); ++i)
            mainwindow->add_file_to_list(list[i].fileName(), list[i].absolutePath(), policy, display_xslt, verbosity);
        return 0;
    }

    void WebPage::close_all()
    {
        mainwindow->clear_file_list();
    }

    void WebPage::close_element(const QString& file)
    {
        mainwindow->remove_file_to_list(file);
    }

    bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest& request,
                                          QWebPage::NavigationType type)
    {
        if (type == QWebPage::NavigationTypeFormSubmitted || type == QWebPage::NavigationTypeFormResubmitted)
            return false;
        return QWebPage::acceptNavigationRequest(frame, request, type);
    }

    QString WebPage::chooseFile(QWebFrame *, const QString& suggested)
    {
        QString value_input;
        if (select_file_name == "checkerRepository_directory")
            value_input = QFileDialog::getExistingDirectory(view(), NULL, suggested);
        else
            value_input = QFileDialog::getOpenFileName(view(), NULL, suggested);

        QMap<QString, QStringList>::iterator it = file_selector.find(select_file_name);
        if (!value_input.length())
        {
            if (it != file_selector.end())
                file_selector.erase(it);
            return QString();
        }
        if (it != file_selector.end())
            file_selector[select_file_name] << value_input;
        else
            file_selector.insert(select_file_name, QStringList(value_input));

        return value_input;
    }

    bool WebPage::extension(Extension extension, const ExtensionOption *option,
                            ExtensionReturn *output)
    {
        if (extension == QWebPage::ChooseMultipleFilesExtension)
        {
            QStringList suggested = ((const ChooseMultipleFilesExtensionOption*)option)->suggestedFileNames;
            QStringList names = QFileDialog::getOpenFileNames(view(), QString::null);
            ((ChooseMultipleFilesExtensionReturn*)output)->fileNames = names;
            QMap<QString, QStringList>::iterator it = file_selector.find("checkerUpload_file");
            if (it != file_selector.end())
                file_selector["checkerUpload_file"] << names;
            else
                file_selector.insert("checkerUpload_file", names);
            return true;
        }
        return false;
    }

    bool WebPage::supportsExtension(Extension extension) const
    {
        if (extension == ChooseMultipleFilesExtension)
            return true;
        return false;
    }

    // TODO
    void WebPage::changeLocalFiles(QStringList& files)
    {
        QWebFrame* frame = mainFrame();
        QString active(" active");

        QWebElement form = frame->findFirstElement("#url");
        if (!form.isNull())
        {
            QString attr = form.attribute("class");
            int pos = -1;
            if ((pos = attr.indexOf(active)) != -1)
            {
                attr.replace(pos, active.length(), "");
                form.setAttribute("class", attr);
            }
        }

        form = frame->findFirstElement("#repository");
        if (!form.isNull())
        {
            QString attr = form.attribute("class");
            int pos = -1;
            if ((pos = attr.indexOf(active)) != -1)
            {
                attr.replace(pos, active.length(), "");
                form.setAttribute("class", attr);
            }
        }

        form = frame->findFirstElement("#file");
        if (form.isNull())
            return;

        QString attr = form.attribute("class");
        if (attr.indexOf(active) == -1)
            form.setAttribute("class", attr + active);

        QWebElement input = form.findFirst("#checkerUpload_file");
        if (input.isNull())
            return;

        QString file = files[0];
        input.setAttribute("value", file);
        QMap<QString, QStringList>::iterator it = file_selector.find("checkerUpload_file");
        if (it != file_selector.end())
            file_selector["checkerUpload_file"] << files;
        else
            file_selector.insert("checkerUpload_file", files);

        onFileUploadSelected(form);
    }

    void WebPage::use_javascript(const QString& js)
    {
        mainFrame()->evaluateJavaScript(js);
    }

    //---------------------------------------------------------------------------
    bool WebPage::report_is_html(const QString& report)
    {
        QRegExp reg("<\\!DOCTYPE.*html", Qt::CaseInsensitive);

        if (reg.indexIn(report, 0) != -1)
            return true;

        return false;
    }

    //---------------------------------------------------------------------------
    bool WebPage::report_is_xml(const QString& report)
    {
        QRegExp reg("<\\?xml ", Qt::CaseInsensitive);

        if (reg.indexIn(report, 0) != -1)
            return true;

        return false;
    }

    //---------------------------------------------------------------------------
    void WebPage::update_status_registered_file(FileRegistered* file)
    {
        set_analyzed_status(file);
        set_implementation_status(file);
        set_policy_status(file);
        delete file;
    }

    //---------------------------------------------------------------------------
    void WebPage::emit_update_registered_file(FileRegistered* file)
    {
        Q_EMIT update_registered_file(file);
    }

    //---------------------------------------------------------------------------
    void WebPage::set_analyzed_status(FileRegistered* file)
    {
        QWebElement status = currentFrame()->findFirstElement(QString("#analyzeStatus%1").arg(file->index));
        QWebElement percent = currentFrame()->findFirstElement(QString("#analyzePercent%1").arg(file->index));

        if (file->analyzed)
        {
            status.setAttribute("class", "success");
            percent.setPlainText("Analyzed");
        }
        else
        {
            status.setAttribute("class", "info");
            QString percent_str;
            if (file->analyze_percent == 0)
                percent_str = "In queue";
            else
                percent_str = "Analyzing";
            percent.setPlainText(percent_str);
        }
    }

    //---------------------------------------------------------------------------
    void WebPage::set_implementation_status(FileRegistered* file)
    {
        QWebElement status = currentFrame()->findFirstElement(QString("#implementationStatus%1").arg(file->index));
        QWebElement viewIcon = currentFrame()->findFirstElement(QString("#implementationStatusViewIcon%1").arg(file->index));
        QWebElement downloadIcon = currentFrame()->findFirstElement(QString("#implementationStatusDownloadIcon%1").arg(file->index));
        QWebElement verbosity_list = currentFrame()->findFirstElement(QString("#fileDetail_implementation_verbosity_list%1").arg(file->index));
        QWebElement displays_list = currentFrame()->findFirstElement(QString("#fileDetail_implementation_displays_list%1").arg(file->index));

        if (file->analyzed)
        {
            downloadIcon.setAttribute("class", "glyphicon glyphicon-download");
            viewIcon.setAttribute("class", "glyphicon glyphicon-eye-open");
            QString html = status.toInnerXml();
            if (file->implementation_valid)
            {
                status.setAttribute("class", "success");
                QString newHtml("<span class=\"glyphicon glyphicon-ok text-success\" aria-hidden=\"true\"></span> Valid");
                status.setInnerXml(newHtml + html);
            }
            else
            {
                status.setAttribute("class", "danger");
                QString newHtml("<span class=\"glyphicon glyphicon-remove\" aria-hidden=\"true\"></span> Not Valid");
                status.setInnerXml(newHtml + html);
            }

            if (file->report_kind != MediaConchLib::report_MediaConch)
            {
                verbosity_list.setAttribute("class", "hidden");
                displays_list.setAttribute("class", "hidden");
            }
            else
            {
                verbosity_list.setAttribute("class", "");
                displays_list.setAttribute("class", "");
            }
        }
        else
        {
            status.setAttribute("class", "info");
            downloadIcon.setAttribute("class", "hidden");
            viewIcon.setAttribute("class", "hidden");
            verbosity_list.setAttribute("class", "hidden");
            displays_list.setAttribute("class", "hidden");
        }
    }

    //---------------------------------------------------------------------------
    void WebPage::set_policy_status(FileRegistered* file)
    {
        QString state("info");
        QWebElement status = currentFrame()->findFirstElement(QString("#policyStatus%1").arg(file->index));
        QWebElement viewIcon = currentFrame()->findFirstElement(QString("#policyStatusViewIcon%1").arg(file->index));
        QWebElement downloadIcon = currentFrame()->findFirstElement(QString("#policyStatusDownloadIcon%1").arg(file->index));
        QWebElement policies_list = currentFrame()->findFirstElement(QString("#fileDetail_policy_policies_list%1").arg(file->index));
        QWebElement displays_list = currentFrame()->findFirstElement(QString("#fileDetail_policy_displays_list%1").arg(file->index));
        QWebElement policy_name = currentFrame()->findFirstElement(QString("#policyElementName%1").arg(file->index));

        if (file->analyzed && file->report_kind == MediaConchLib::report_MediaConch)
        {
            downloadIcon.setAttribute("class", "glyphicon glyphicon-download");
            viewIcon.setAttribute("class", "glyphicon glyphicon-eye-open");

            if (file->policy != -1)
            {
                state = file->policy_valid ? "success" : "danger";
                status.setAttribute("class", state);
                QString html = status.toInnerXml();
                if (file->policy_valid)
                {
                    QString newHtml("<span class=\"glyphicon glyphicon-ok text-success\" aria-hidden=\"true\"></span> ");
                    status.setInnerXml(newHtml + html);
                }
                else
                {
                    QString newHtml("<span class=\"glyphicon glyphicon-remove\" aria-hidden=\"true\"></span> ");
                    status.setInnerXml(newHtml + html);
                }
            }
        }
        else
        {
            status.setAttribute("class", state);
            downloadIcon.setAttribute("class", "hidden");
            viewIcon.setAttribute("class", "hidden");
        }

        if (file->analyzed && file->report_kind != MediaConchLib::report_MediaConch)
        {
            policies_list.setAttribute("class", "hidden");
            displays_list.setAttribute("class", "hidden");
            policy_name.setInnerXml("N/A");
        }
    }

}

#endif
