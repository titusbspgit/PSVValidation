# PCIE TestPlan Excel Generation (Fallback Automation)

This repository now contains a deterministic fallback workflow to generate and commit the PCIE TestPlan Excel file from embedded JSON when direct in-session XLSX generation/commit is unavailable.

Files added:
- .github/workflows/generate_pcie_testplan.yml — GitHub Action that builds the Excel on push
- scripts/gen_pcie_testplan.py — Python script that converts embedded JSON to a formatted .xlsx following strict rules and commits it

Output location:
- Test_Output/PCIE/TestPlan/PCIE_TestPlan_YYYYMMDD_HHMMSS.xlsx

Notes:
- The workflow triggers when these files change. To re-generate, update the script or workflow and push.
- IST timestamp is embedded in the filename and commit message.
- The Meta_data_sheet is marked Very Hidden and the main visible sheet is TestPlan (no Data sheet remains).
