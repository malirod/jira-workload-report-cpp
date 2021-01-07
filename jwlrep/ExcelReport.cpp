// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <jwlrep/AppConfig.h>
#include <jwlrep/ExcelReport.h>
#include <jwlrep/Logger.h>
#include <jwlrep/Worklog.h>

#include <boost/algorithm/string.hpp>
#include <xlnt/xlnt.hpp>

namespace {

std::size_t const kHeaderRow = 1U;
std::size_t const kColumnIndexKey = 1U;
std::size_t const kColumnIndexSummary = 2U;
std::size_t const kColumnIndexAuthor = 3U;
std::size_t const kColumnIndexDate = 4U;
std::size_t const kColumnIndexSpent = 5U;
std::size_t const kColumnIndexLabel = 6U;
std::size_t const kColumnIndexProject = 7U;
std::size_t const kColumnIndexCommon = 8U;
std::size_t const kColumnIndexArch = 9U;

char const *const kHeaderProject = "Project (h)";
char const *const kHeaderCommon = "Common (h)";
char const *const kHeaderArch = "Arch (h)";

void addHeadingToReport(xlnt::worksheet &worksheet) {
  worksheet.cell(xlnt::cell_reference(kColumnIndexKey, kHeaderRow))
      .value("Key");
  worksheet.cell(xlnt::cell_reference(kColumnIndexSummary, kHeaderRow))
      .value("Summary");
  worksheet.cell(xlnt::cell_reference(kColumnIndexAuthor, kHeaderRow))
      .value("Author");
  worksheet.cell(xlnt::cell_reference(kColumnIndexDate, kHeaderRow))
      .value("Date");
  worksheet.cell(xlnt::cell_reference(kColumnIndexSpent, kHeaderRow))
      .value("Spent (h)");
  worksheet.cell(xlnt::cell_reference(kColumnIndexLabel, kHeaderRow))
      .value("Label");
  worksheet.cell(xlnt::cell_reference(kColumnIndexProject, kHeaderRow))
      .value(kHeaderProject);
  worksheet.cell(xlnt::cell_reference(kColumnIndexCommon, kHeaderRow))
      .value(kHeaderCommon);
  worksheet.cell(xlnt::cell_reference(kColumnIndexArch, kHeaderRow))
      .value(kHeaderArch);
}

auto calculateLabel(std::string const &summary) -> std::string {
  using boost::algorithm::contains;
  using boost::algorithm::to_lower_copy;

  if (contains(to_lower_copy(summary), "[common]")) {
    return "Common";
  }
  if (contains(to_lower_copy(summary), "[arch]")) {
    return "Arch";
  }
  if (contains(to_lower_copy(summary), "overtime")) {
    return "Overtime";
  }
  if (contains(to_lower_copy(summary), "vacation")) {
    return "Vacation";
  }
  if (contains(to_lower_copy(summary), "Sick leaves")) {
    return "Sick leaves";
  }
  return "2520";
}

void addWorklogSummaryToWorksheet(xlnt::worksheet &worksheet,
                                  std::vector<jwlrep::Worklog> const &worklog,
                                  std::int32_t lastRowIndex) {
  worksheet.cell(xlnt::cell_reference(kColumnIndexKey, lastRowIndex))
      .value("Total");
  worksheet.cell(xlnt::cell_reference(kColumnIndexKey, lastRowIndex + 1))
      .value(kHeaderProject);
  worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 1, lastRowIndex + 1))
      .value(kHeaderCommon);
  worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 2, lastRowIndex + 1))
      .value(kHeaderArch);

  auto const isHasWorklogItems = !worklog.empty();
  if (isHasWorklogItems) {
    worksheet.cell(xlnt::cell_reference(kColumnIndexSpent, lastRowIndex))
        .formula(fmt::format("=SUM(E2:E{})", lastRowIndex - 1));
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey, lastRowIndex + 2))
        .formula(fmt::format("=SUM(G2:G{})", lastRowIndex - 1));
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 1, lastRowIndex + 2))
        .formula(fmt::format("=SUM(H2:H{})", lastRowIndex - 1));
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 2, lastRowIndex + 2))
        .formula(fmt::format("=SUM(I2:I{})", lastRowIndex - 1));
  } else {
    worksheet.cell(xlnt::cell_reference(kColumnIndexSpent, lastRowIndex + 1))
        .value(0);
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey, lastRowIndex + 2))
        .value(0);
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 1, lastRowIndex + 2))
        .value(0);
    worksheet.cell(xlnt::cell_reference(kColumnIndexKey + 2, lastRowIndex + 2))
        .value(0);
  }
}

void addWorklogToWorksheet(xlnt::worksheet &worksheet,
                           std::vector<jwlrep::Worklog> const &worklog,
                           jwlrep::Options const & /*unused*/) {
  std::uint32_t rowIndex = kHeaderRow + 1U;
  for (auto const &issue : worklog) {
    for (auto const &entry : issue.entries()) {
      worksheet.cell(xlnt::cell_reference(kColumnIndexKey, rowIndex))
          .value(issue.key());
      worksheet.cell(xlnt::cell_reference(kColumnIndexSummary, rowIndex))
          .value(issue.summary());
      worksheet.cell(xlnt::cell_reference(kColumnIndexAuthor, rowIndex))
          .value(entry.author());
      worksheet.cell(xlnt::cell_reference(kColumnIndexDate, rowIndex))
          .value(boost::gregorian::to_iso_extended_string(entry.created()));
      auto const kMSecsPerHour = 3600;
      worksheet.cell(xlnt::cell_reference(kColumnIndexSpent, rowIndex))
          .value(entry.timeSpent().count() / kMSecsPerHour);
      worksheet.cell(xlnt::cell_reference(kColumnIndexLabel, rowIndex))
          .value(calculateLabel(issue.summary()));
      worksheet.cell(xlnt::cell_reference(kColumnIndexProject, rowIndex))
          .formula(fmt::format("=IF(F{0}=\"2520\",E{0},0)", rowIndex + 1));
      worksheet.cell(xlnt::cell_reference(kColumnIndexCommon, rowIndex))
          .formula(fmt::format("=IF(F{0}=\"Common\",E{0},0)", rowIndex + 1));
      worksheet.cell(xlnt::cell_reference(kColumnIndexArch, rowIndex))
          .formula(fmt::format("=IF(F{0}=\"Arch\",E{0},0)", rowIndex + 1));
      ++rowIndex;
    }
  }
  addWorklogSummaryToWorksheet(worksheet, worklog, rowIndex);
}

void addTimeSheetsToReport(xlnt::workbook &workbook,
                           jwlrep::TimeSheets const &timeSheets,
                           jwlrep::Options const &options) {
  for (auto const &userTimeSheetRef : timeSheets) {
    if (userTimeSheetRef.worklog().empty() ||
        userTimeSheetRef.worklog()[0].entries().empty()) {
      LOG_INFO("No data to save to the report");
      continue;
    }

    auto worksheet = workbook.create_sheet();
    worksheet.title(userTimeSheetRef.worklog()[0].entries()[0].author());

    addHeadingToReport(worksheet);
    addWorklogToWorksheet(worksheet, userTimeSheetRef.worklog(), options);
  }
}

}  // namespace

namespace jwlrep {

void createReportExcel(TimeSheets const &timeSheets, Options const &options) {
  xlnt::workbook workbook;
  workbook.active_sheet().title("Summary");

  addTimeSheetsToReport(workbook, timeSheets, options);

  workbook.save("report.xlsx");
}

}  // namespace jwlrep