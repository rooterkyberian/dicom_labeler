dicom_labeler
=============

Tool for overlaying DICOM-embedded images with custom labels.

Label Templates are made as HTML files and processed by QtWebKit. DICOM tags noted as `#(CommonName)' or `#(0x0000,0x0000)' are replaced (currently by simple regexp) to the value found in the DICOM metadata.

Current state
----
* template rendering works (QtWebkit; code based on http://cutycapt.sourceforge.net/ )
* metadata and image extraction (dcmtk) - support for different image formats probably needs more work
* still no support for writing back to the DICOM file (or copy of it)

Example files on which this tool has been tested can be found here: http://www.insight-journal.org/midas/collection/view/194


