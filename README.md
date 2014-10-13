dicom_labeler
=============

Tool for overlaying DICOM-embedded images with custom labels.

Label Templates are made as HTML files and processed by QtWebKit. DICOM tags noted as `#(CommonName)' or `#(0x0000,0x0000)' are replaced (currently by simple regexp) to the value found in the DICOM metadata.

Current state
----
* template rendering works (QtWebkit; code based on http://cutycapt.sourceforge.net/ )
* metadata extraction (dcmtk)
* DICOM export (dcmtk) by in-place editing. Only non-compressed Transfer syntax and monochrome interpolation. RGB may or may not work.

Example files on which this tool has been tested can be found here: http://www.insight-journal.org/midas/collection/view/194


