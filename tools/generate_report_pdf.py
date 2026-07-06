from pathlib import Path
import re

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    SimpleDocTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
    PageBreak,
)


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "组号-作业报告.pdf"
MD = ROOT / "docs" / "final_report.md"
FONT = Path(r"C:\Windows\Fonts\NotoSansSC-VF.ttf")
BOLD_FONT = Path(r"C:\Windows\Fonts\Dengb.ttf")


def register_fonts():
    pdfmetrics.registerFont(TTFont("CN", str(FONT)))
    pdfmetrics.registerFont(TTFont("CN-Bold", str(BOLD_FONT)))


def esc(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
    )


def parse_markdown(md: str):
    lines = md.splitlines()
    blocks = []
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        if not line:
            i += 1
            continue
        if line.startswith("# "):
            blocks.append(("title", line[2:].strip()))
            i += 1
        elif line.startswith("## "):
            blocks.append(("h2", line[3:].strip()))
            i += 1
        elif line.startswith("### "):
            blocks.append(("h3", line[4:].strip()))
            i += 1
        elif line.startswith("> "):
            blocks.append(("quote", line[2:].strip()))
            i += 1
        elif line.startswith("| "):
            rows = []
            while i < len(lines) and lines[i].startswith("| "):
                cells = [c.strip() for c in lines[i].strip("|").split("|")]
                if not all(set(c) <= {"-", " "} for c in cells):
                    rows.append(cells)
                i += 1
            blocks.append(("table", rows))
        else:
            para = [line]
            i += 1
            while i < len(lines) and lines[i].strip() and not lines[i].startswith(("#", "|", "> ")):
                para.append(lines[i].strip())
                i += 1
            blocks.append(("p", " ".join(para)))
    return blocks


def on_page(canvas, doc):
    page_num = canvas.getPageNumber()
    canvas.saveState()
    canvas.setFont("CN", 9)
    canvas.setFillColor(colors.HexColor("#777777"))
    canvas.drawCentredString(A4[0] / 2, 12 * mm, f"第 {page_num} 页")
    canvas.restoreState()


def main():
    register_fonts()
    styles = getSampleStyleSheet()
    title = ParagraphStyle(
        "title",
        parent=styles["Title"],
        fontName="CN-Bold",
        fontSize=20,
        leading=28,
        alignment=TA_CENTER,
        textColor=colors.HexColor("#8c1515"),
        spaceAfter=10,
    )
    h2 = ParagraphStyle(
        "h2",
        parent=styles["Heading2"],
        fontName="CN-Bold",
        fontSize=14,
        leading=20,
        textColor=colors.HexColor("#8c1515"),
        spaceBefore=8,
        spaceAfter=5,
    )
    h3 = ParagraphStyle(
        "h3",
        parent=styles["Heading3"],
        fontName="CN-Bold",
        fontSize=11,
        leading=16,
        textColor=colors.HexColor("#333333"),
        spaceBefore=5,
        spaceAfter=3,
    )
    body = ParagraphStyle(
        "body",
        parent=styles["BodyText"],
        fontName="CN",
        fontSize=9,
        leading=14,
        firstLineIndent=18,
        alignment=TA_LEFT,
        spaceAfter=4,
    )
    quote = ParagraphStyle(
        "quote",
        parent=body,
        firstLineIndent=0,
        leftIndent=8,
        textColor=colors.HexColor("#8c1515"),
        backColor=colors.HexColor("#fff7f7"),
        borderColor=colors.HexColor("#f1d7d7"),
        borderWidth=0.5,
        borderPadding=5,
    )

    story = []
    for kind, value in parse_markdown(MD.read_text(encoding="utf-8")):
        if kind == "title":
            story.append(Paragraph(esc(value), title))
            story.append(Spacer(1, 5 * mm))
        elif kind == "h2":
            story.append(Paragraph(esc(value), h2))
        elif kind == "h3":
            story.append(Paragraph(esc(value), h3))
        elif kind == "p":
            text = re.sub(r"`([^`]+)`", r"<font name='CN-Bold'>\1</font>", esc(value))
            story.append(Paragraph(text, body))
        elif kind == "quote":
            story.append(Paragraph(esc(value), quote))
        elif kind == "table":
            table_data = [[Paragraph(esc(cell), ParagraphStyle("cell", fontName="CN", fontSize=8, leading=12)) for cell in row] for row in value]
            table = Table(table_data, colWidths=[35 * mm, 120 * mm])
            table.setStyle(TableStyle([
                ("FONTNAME", (0, 0), (-1, -1), "CN"),
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#f8eeee")),
                ("TEXTCOLOR", (0, 0), (-1, 0), colors.HexColor("#8c1515")),
                ("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#dddddd")),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 6),
                ("RIGHTPADDING", (0, 0), (-1, -1), 6),
                ("TOPPADDING", (0, 0), (-1, -1), 5),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
            ]))
            story.append(table)
            story.append(Spacer(1, 3 * mm))

    doc = SimpleDocTemplate(
        str(OUT),
        pagesize=A4,
        rightMargin=17 * mm,
        leftMargin=17 * mm,
        topMargin=16 * mm,
        bottomMargin=18 * mm,
        title="作业报告",
    )
    doc.build(story, onFirstPage=on_page, onLaterPages=on_page)
    print(OUT)


if __name__ == "__main__":
    main()
