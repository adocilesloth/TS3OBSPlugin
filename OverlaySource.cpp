/********************************************************************************
Adapted from TextOutputSource.cpp origionally created by <obs.jim@gmail.com>
********************************************************************************/
#include "OBSApi.h"
#include "TS3Plugin.h"

#include <memory>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>

#include <fstream>

HINSTANCE OvrHinst;

//colours for the dialog box so they are initialised 
DWORD fcolour  = 161235 | 0xFFFFFFFF;
DWORD bgcolour = 161235 | 0xFFFFFFFF;
DWORD olcolour = 161235 | 0xFFFFFFFF;
int iname = 10;
bool bname = false;
bool bright = false;

#define ClampVal(val, minVal, maxVal) \
    if(val < minVal) val = minVal; \
    else if(val > maxVal) val = maxVal;

inline DWORD GetAlphaVal(UINT opacityLevel)
{
    return ((opacityLevel*255/100)&0xFF) << 24;
}

class OverlaySource : public ImageSource
{
	 bool        bUpdateTexture;

    String      strCurrentText;
    Texture     *texture;
    float       scrollValue;
    float       showExtentTime;

    int         mode;
    String      strText;
    String      strFile;

    String      strFont;
    int         size;
    DWORD       color;
    UINT        opacity;
    UINT        globalOpacity;
    bool        bBold, bItalic, bUnderline, bVertical;

    UINT        backgroundOpacity;
    DWORD       backgroundColor;

    bool        bUseOutline;
    float       outlineSize;
    DWORD       outlineColor;
    UINT        outlineOpacity;

    bool        bUseExtents;
    UINT        extentWidth, extentHeight;

    bool        bWrap;
    int         align;

    Vect2       baseSize;
    SIZE        textureSize;
    bool        bUsePointFiltering;

	//name max
	int			nameNumber;
	bool		bHideName;
	bool		bRightSymbol;

    bool        bMonitoringFileChanges;
    OSFileChangeData *fileChangeMonitor;

    std::unique_ptr<SamplerState> sampler;

    bool        bDoUpdate;

    SamplerState *ss;

    XElement    *data;

	void SetFile()
	{
		std::wstring path = OBSGetPluginDataPath().Array();
		path.append(L"\\Overlay.txt");
		strFile = path.c_str();
		mode = 1;
	}

    void DrawOutlineText(Gdiplus::Graphics *graphics,
                         Gdiplus::Font &font,
                         const Gdiplus::GraphicsPath &path,
                         const Gdiplus::StringFormat &format,
                         const Gdiplus::Brush *brush)
    {
                
        Gdiplus::GraphicsPath *outlinePath;

        outlinePath = path.Clone();

        // Outline color and size
        UINT tmpOpacity = (UINT)((((float)opacity * 0.01f) * ((float)outlineOpacity * 0.01f)) * 100.0f);
        Gdiplus::Pen pen(Gdiplus::Color(GetAlphaVal(tmpOpacity) | (outlineColor&0xFFFFFF)), outlineSize);
        pen.SetLineJoin(Gdiplus::LineJoinRound);

        // Draw the outline
        graphics->DrawPath(&pen, outlinePath);

        // Draw the text        
        graphics->FillPath(brush, &path);

        delete outlinePath;
    }

    HFONT GetFont()
    {
        HFONT hFont = NULL;

        LOGFONT lf;
        zero(&lf, sizeof(lf));
        lf.lfHeight = size;
        lf.lfWeight = bBold ? FW_BOLD : FW_DONTCARE;
        lf.lfItalic = bItalic;
        lf.lfUnderline = bUnderline;
        lf.lfQuality = ANTIALIASED_QUALITY;

        if(strFont.IsValid())
        {
            scpy_n(lf.lfFaceName, strFont, 31);

            hFont = CreateFontIndirect(&lf);
        }

        if(!hFont)
        {
            scpy_n(lf.lfFaceName, TEXT("Arial"), 31);
            hFont = CreateFontIndirect(&lf);
        }

        return hFont;
    }

    void UpdateCurrentText()
    {
        if(bMonitoringFileChanges)
        {
            OSMonitorFileDestroy(fileChangeMonitor);
            fileChangeMonitor = NULL;

            bMonitoringFileChanges = false;
        }

        if(mode == 0)
            strCurrentText = strText;

        else if(mode == 1 && strFile.IsValid())
        {
            XFile textFile;
            if(textFile.Open(strFile, XFILE_READ | XFILE_SHARED, XFILE_OPENEXISTING))
            {
                textFile.ReadFileToString(strCurrentText);
            }
            else
            {
                strCurrentText = TEXT("");
                AppWarning(TEXT("OverlaySource::UpdateTexture: could not open specified file (invalid file name or access violation)"));
            }

            if (fileChangeMonitor = OSMonitorFileStart (strFile))
                bMonitoringFileChanges = true;
        }
        else
            strCurrentText = TEXT("");
    }

	void SetStringFormat(Gdiplus::StringFormat &format)
    {
        UINT formatFlags;

        formatFlags = Gdiplus::StringFormatFlagsNoFitBlackBox
                    | Gdiplus::StringFormatFlagsMeasureTrailingSpaces;


        if(bVertical)
            formatFlags |= Gdiplus::StringFormatFlagsDirectionVertical
                         | Gdiplus::StringFormatFlagsDirectionRightToLeft;

        format.SetFormatFlags(formatFlags);
        format.SetTrimming(Gdiplus::StringTrimmingWord);

        if(bUseExtents && bWrap)
            switch(align)
            {
                case 0:
                    if(bVertical)
                        format.SetLineAlignment(Gdiplus::StringAlignmentFar);
                    else
                        format.SetAlignment(Gdiplus::StringAlignmentNear);
                    break;
                case 1:
                    if(bVertical)
                        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
                    else
                        format.SetAlignment(Gdiplus::StringAlignmentCenter);
                    break;
                case 2:
                    if(bVertical)
                        format.SetLineAlignment(Gdiplus::StringAlignmentNear);
                    else
                        format.SetAlignment(Gdiplus::StringAlignmentFar);
                    break;
            }
        else if(bUseExtents && bVertical && !bWrap)
                format.SetLineAlignment(Gdiplus::StringAlignmentFar);
        else if(bVertical)
                format.SetLineAlignment(Gdiplus::StringAlignmentFar);

    }

	void UpdateTexture()
    {
        HFONT hFont;
        Gdiplus::Status stat;
        Gdiplus::RectF layoutBox;
        SIZE textSize;

        Gdiplus::RectF boundingBox(0.0f, 0.0f, 32.0f, 32.0f);

        UpdateCurrentText();

        hFont = GetFont();
        if(!hFont)
            return;

        Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());

        SetStringFormat(format);

        HDC hdc = CreateCompatibleDC(NULL);

        Gdiplus::Font font(hdc, hFont);
        Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);

        graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

        if(strCurrentText.IsValid())
        {
            if(bUseExtents && bWrap)
            {
                layoutBox.X = layoutBox.Y = 0;
                layoutBox.Width  = float(extentWidth);
                layoutBox.Height = float(extentHeight);

                if(bUseOutline)
                {
                    //Note: since there's no path widening in DrawOutlineText the padding is half than what it was supposed to be.
                    layoutBox.Width  -= outlineSize;
                    layoutBox.Height -= outlineSize;
                }


                stat = graphics->MeasureString(strCurrentText, -1, &font, layoutBox, &format, &boundingBox);
                if(stat != Gdiplus::Ok)
                    AppWarning(TEXT("OverlaySource::UpdateTexture: Gdiplus::Graphics::MeasureString failed: %u"), (int)stat);
            }
            else
            {
                stat = graphics->MeasureString(strCurrentText, -1, &font, Gdiplus::PointF(0.0f, 0.0f), &format, &boundingBox);
                if(stat != Gdiplus::Ok)
                    AppWarning(TEXT("OverlaySource::UpdateTexture: Gdiplus::Graphics::MeasureString failed: %u"), (int)stat);
                if(bUseOutline)
                {
                    //Note: since there's no path widening in DrawOutlineText the padding is half than what it was supposed to be.
                    boundingBox.Width  += outlineSize;
                    boundingBox.Height += outlineSize;
				}
			}
        }

        delete graphics;

        DeleteDC(hdc);
        hdc = NULL;
        DeleteObject(hFont);

        if(bVertical)
        {
            if(boundingBox.Width<size)
            {
                textSize.cx = size;
                boundingBox.Width = float(size);
            }
            else
                textSize.cx = LONG(boundingBox.Width + EPSILON);

            textSize.cy = LONG(boundingBox.Height + EPSILON);
        }
        else
        {
            if(boundingBox.Height<size)
            {
                textSize.cy = size;
                boundingBox.Height = float(size);
            }
            else
                textSize.cy = LONG(boundingBox.Height + EPSILON);

            textSize.cx = LONG(boundingBox.Width + EPSILON);
        }

        if(bUseExtents)
        {
            if(bWrap)
            {
                textSize.cx = extentWidth;
                textSize.cy = extentHeight;
            }
            else
            {
                if(LONG(extentWidth) > textSize.cx)
                    textSize.cx = extentWidth;
                if(LONG(extentHeight) > textSize.cy)
                    textSize.cy = extentHeight;
            }
        }

        textSize.cx += textSize.cx%2;
        textSize.cy += textSize.cy%2;

        ClampVal(textSize.cx, 32, 8192);
        ClampVal(textSize.cy, 32, 8192);

        //----------------------------------------------------------------------
        // write image

        {
            HDC hTempDC = CreateCompatibleDC(NULL);

            BITMAPINFO bi;
            zero(&bi, sizeof(bi));

            void* lpBits;

            BITMAPINFOHEADER &bih = bi.bmiHeader;
            bih.biSize = sizeof(bih);
            bih.biBitCount = 32;
            bih.biWidth  = textSize.cx;
            bih.biHeight = textSize.cy;
            bih.biPlanes = 1;

            HBITMAP hBitmap = CreateDIBSection(hTempDC, &bi, DIB_RGB_COLORS, &lpBits, NULL, 0);

            Gdiplus::Bitmap      bmp(textSize.cx, textSize.cy, 4*textSize.cx, PixelFormat32bppARGB, (BYTE*)lpBits);

            graphics = new Gdiplus::Graphics(&bmp); 

            Gdiplus::SolidBrush  *brush = new Gdiplus::SolidBrush(Gdiplus::Color(GetAlphaVal(opacity)|(color&0x00FFFFFF)));

            DWORD bkColor;

            bkColor = ((strCurrentText.IsValid() || bUseExtents) ? GetAlphaVal(backgroundOpacity) : GetAlphaVal(0)) | (backgroundColor&0x00FFFFFF);

            if((textSize.cx > boundingBox.Width  || textSize.cy > boundingBox.Height) && !bUseExtents)
            {
                stat = graphics->Clear(Gdiplus::Color( 0x00000000));
                if(stat != Gdiplus::Ok)
                    AppWarning(TEXT("OverlaySource::UpdateTexture: Graphics::Clear failed: %u"), (int)stat);

                Gdiplus::SolidBrush *bkBrush = new Gdiplus::SolidBrush(Gdiplus::Color( bkColor ));

                graphics->FillRectangle(bkBrush, boundingBox);

                delete bkBrush;
            }
            else
            {
                stat = graphics->Clear(Gdiplus::Color( bkColor ));
                if(stat != Gdiplus::Ok)
                    AppWarning(TEXT("OverlaySource::UpdateTexture: Graphics::Clear failed: %u"), (int)stat);
            }

            graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
            graphics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
            graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            
            if(strCurrentText.IsValid())
            {
                if(bUseOutline)
                {
                    boundingBox.Offset(outlineSize/2, outlineSize/2);

                    Gdiplus::FontFamily fontFamily;
                    Gdiplus::GraphicsPath path;

                    font.GetFamily(&fontFamily);

                    path.AddString(strCurrentText, -1, &fontFamily, font.GetStyle(), font.GetSize(), boundingBox, &format);

                    DrawOutlineText(graphics, font, path, format, brush);
                }
                else
                {
                    stat = graphics->DrawString(strCurrentText, -1, &font, boundingBox, &format, brush);
                    if(stat != Gdiplus::Ok)
                        AppWarning(TEXT("OverlaySource::UpdateTexture: Graphics::DrawString failed: %u"), (int)stat);
                }
            }

            delete brush;
            delete graphics;

            //----------------------------------------------------------------------
            // upload texture

            if(textureSize.cx != textSize.cx || textureSize.cy != textSize.cy)
            {
                if(texture)
                {
                    delete texture;
                    texture = NULL;
                }

                mcpy(&textureSize, &textSize, sizeof(textureSize));
                texture = CreateTexture(textSize.cx, textSize.cy, GS_BGRA, lpBits, FALSE, FALSE);
            }
            else if(texture)
                texture->SetImage(lpBits, GS_IMAGEFORMAT_BGRA, 4*textSize.cx);

            if(!texture)
            {
                AppWarning(TEXT("OverlaySource::UpdateTexture: could not create texture"));
                DeleteObject(hFont);
            }

            DeleteDC(hTempDC);
            DeleteObject(hBitmap);
        }
    }

public:
    inline OverlaySource(XElement *data)
    {
        this->data = data;
        UpdateSettings();
		SetFile();

		data->SetInt(TEXT("nameNumber"), iname);
		data->SetInt(TEXT("bHideName"), bname);

        SamplerInfo si;
        zero(&si, sizeof(si));
        si.addressU = GS_ADDRESS_REPEAT;
        si.addressV = GS_ADDRESS_REPEAT;
        si.borderColor = 0;
        si.filter = GS_FILTER_LINEAR;
        ss = CreateSamplerState(si);
        globalOpacity = 100;

        Log(TEXT("Using overlay output"));
    }

    ~OverlaySource()
    {
        if(texture)
        {
            delete texture;
            texture = NULL;
        }

        delete ss;

        if(bMonitoringFileChanges)
        {
            OSMonitorFileDestroy(fileChangeMonitor);
        }
    }

	void Preprocess()
    {
        if(bMonitoringFileChanges)
        {
            if (OSFileHasChanged(fileChangeMonitor))
                bUpdateTexture = true;
        }

        if(bUpdateTexture)
        {
            bUpdateTexture = false;
            UpdateTexture();
        }
    }

	 void Render(const Vect2 &pos, const Vect2 &size)
    {
        if(texture)
        {

            Vect2 sizeMultiplier = size/baseSize;
            Vect2 newSize = Vect2(float(textureSize.cx), float(textureSize.cy))*sizeMultiplier;

            if(bUseExtents)
            {
                Vect2 extentVal = Vect2(float(extentWidth), float(extentHeight))*sizeMultiplier;
                if(showExtentTime > 0.0f)
                {
                    Shader *pShader = GS->GetCurrentPixelShader();
                    Shader *vShader = GS->GetCurrentVertexShader();

                    Color4 rectangleColor = Color4(0.0f, 1.0f, 0.0f, 1.0f);
                    if(showExtentTime < 1.0f)
                        rectangleColor.w = showExtentTime;

                    DrawBox(pos, extentVal);

                    LoadVertexShader(vShader);
                    LoadPixelShader(pShader);
                }

                if(!bWrap)
                {
                    XRect rect = {int(pos.x), int(pos.y), int(extentVal.x), int(extentVal.y)};
                    SetScissorRect(&rect);
                }
            }

            if(bUsePointFiltering) {
                if (!sampler) {
                    SamplerInfo samplerinfo;
                    samplerinfo.filter = GS_FILTER_POINT;
                    std::unique_ptr<SamplerState> new_sampler(CreateSamplerState(samplerinfo));
                    sampler = std::move(new_sampler);
                }

                LoadSamplerState(sampler.get(), 0);
            }

            DWORD alpha = DWORD(double(globalOpacity)*2.55);
            DWORD outputColor = (alpha << 24) | 0xFFFFFF;

            DrawSprite(texture, outputColor, pos.x, pos.y, pos.x+newSize.x, pos.y+newSize.y);

            if (bUsePointFiltering)
                LoadSamplerState(NULL, 0);

            if(bUseExtents && !bWrap)
                SetScissorRect(NULL);
        }
    }

	 Vect2 GetSize() const
    {
        return baseSize;
    }

    void UpdateSettings()
    {
        strFont     = data->GetString(TEXT("font"), TEXT("Arial"));
        color       = data->GetInt(TEXT("color"), 0xFFFFFFFF);
        size        = data->GetInt(TEXT("fontSize"), 48);
        opacity     = data->GetInt(TEXT("textOpacity"), 100);
        bBold       = data->GetInt(TEXT("bold"), 0) != 0;
        bItalic     = data->GetInt(TEXT("italic"), 0) != 0;
        bWrap       = data->GetInt(TEXT("wrap"), 0) != 0;
        bUnderline  = data->GetInt(TEXT("underline"), 0) != 0;
        bVertical   = data->GetInt(TEXT("vertical"), 0) != 0;
        bUseExtents = data->GetInt(TEXT("useTextExtents"), 0) != 0;
        extentWidth = data->GetInt(TEXT("extentWidth"), 0);
        extentHeight= data->GetInt(TEXT("extentHeight"), 0);
        align       = data->GetInt(TEXT("align"), 0);
        strFile     = data->GetString(TEXT("file"));
        strText     = data->GetString(TEXT("text"));
        mode        = data->GetInt(TEXT("mode"), 1);
        bUsePointFiltering = data->GetInt(TEXT("pointFiltering"), 0) != 0;

        baseSize.x  = data->GetFloat(TEXT("baseSizeCX"), 100);
        baseSize.y  = data->GetFloat(TEXT("baseSizeCY"), 100);

        bUseOutline    = data->GetInt(TEXT("useOutline")) != 0;
        outlineColor   = data->GetInt(TEXT("outlineColor"), 0xFF000000);
        outlineSize    = data->GetFloat(TEXT("outlineSize"), 2);
        outlineOpacity = data->GetInt(TEXT("outlineOpacity"), 100);

        backgroundColor   = data->GetInt(TEXT("backgroundColor"), 0xFF000000);
        backgroundOpacity = data->GetInt(TEXT("backgroundOpacity"), 0);

		nameNumber = data->GetInt(TEXT("nameNumber"), 0);
		iname = data->GetInt(TEXT("nameNumber"), 0);
		bHideName = data->GetInt(TEXT("bHideName"), bname) != 0;
		bname = data->GetInt(TEXT("bHideName"), bname) != 0;
		bright = data->GetInt(TEXT("bRightSymbol"), bright) != 0;

        bUpdateTexture = true;
    }

	void SetString(CTSTR lpName, CTSTR lpVal)
    {
        if(scmpi(lpName, TEXT("font")) == 0)
            strFont = lpVal;
        else if(scmpi(lpName, TEXT("text")) == 0)
            strText = lpVal;
        else if(scmpi(lpName, TEXT("file")) == 0)
            strFile = lpVal;

        bUpdateTexture = true;
    }

    void SetInt(CTSTR lpName, int iValue)
    {
        if(scmpi(lpName, TEXT("color")) == 0)
            color = iValue;
        else if(scmpi(lpName, TEXT("fontSize")) == 0)
            size = iValue;
        else if(scmpi(lpName, TEXT("textOpacity")) == 0)
            opacity = iValue;
        else if(scmpi(lpName, TEXT("bold")) == 0)
            bBold = iValue != 0;
        else if(scmpi(lpName, TEXT("italic")) == 0)
            bItalic = iValue != 0;
        else if(scmpi(lpName, TEXT("wrap")) == 0)
            bWrap = iValue != 0;
        else if(scmpi(lpName, TEXT("underline")) == 0)
            bUnderline = iValue != 0;
        else if(scmpi(lpName, TEXT("vertical")) == 0)
            bVertical = iValue != 0;
        else if(scmpi(lpName, TEXT("useTextExtents")) == 0)
            bUseExtents = iValue != 0;
        else if(scmpi(lpName, TEXT("extentWidth")) == 0)
        {
            showExtentTime = 2.0f;
            extentWidth = iValue;
        }
        else if(scmpi(lpName, TEXT("extentHeight")) == 0)
        {
            showExtentTime = 2.0f;
            extentHeight = iValue;
        }
        else if(scmpi(lpName, TEXT("align")) == 0)
            align = iValue;
        else if(scmpi(lpName, TEXT("mode")) == 0)
            mode = iValue;
        else if(scmpi(lpName, TEXT("useOutline")) == 0)
            bUseOutline = iValue != 0;
        else if(scmpi(lpName, TEXT("outlineColor")) == 0)
            outlineColor = iValue;
        else if(scmpi(lpName, TEXT("outlineOpacity")) == 0)
            outlineOpacity = iValue;
        else if(scmpi(lpName, TEXT("backgroundColor")) == 0)
            backgroundColor = iValue;
        else if(scmpi(lpName, TEXT("backgroundOpacity")) == 0)
            backgroundOpacity = iValue;

        bUpdateTexture = true;
    }

    void SetFloat(CTSTR lpName, float fValue)
    {
        if(scmpi(lpName, TEXT("outlineSize")) == 0)
            outlineSize = fValue;

        bUpdateTexture = true;
    }

    inline void ResetExtentRect() {showExtentTime = 0.0f;}
};

struct ConfigOverlaySourceInfo
{
    CTSTR lpName;
    XElement *data;
    float cx, cy;

    StringList fontNames;
    StringList fontFaces;
};

ImageSource* STDCALL CreateOverlaySource(XElement *data)
{
    if(!data)
        return NULL;
    return new OverlaySource(data);
}

int CALLBACK OvrFontEnumProcThingy(ENUMLOGFONTEX *logicalData, NEWTEXTMETRICEX *physicalData, DWORD fontType, ConfigOverlaySourceInfo *configInfo)
{
    if(fontType == TRUETYPE_FONTTYPE) //HomeWorld - GDI+ doesn't like anything other than truetype
    {
        configInfo->fontNames << logicalData->elfFullName;
        configInfo->fontFaces << logicalData->elfLogFont.lfFaceName;
    }

    return 1;
}

void OvrDoCancelStuff(HWND hwnd)
{
    ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
    ImageSource *source = API->GetSceneImageSource(configInfo->lpName);

    if(source)
        source->UpdateSettings();
}

UINT OvrFindFontFace(ConfigOverlaySourceInfo *configInfo, HWND hwndFontList, CTSTR lpFontFace)
{
    UINT id = configInfo->fontFaces.FindValueIndexI(lpFontFace);
    if(id == INVALID)
        return INVALID;
    else
    {
        for(UINT i=0; i<configInfo->fontFaces.Num(); i++)
        {
            UINT targetID = (UINT)SendMessage(hwndFontList, CB_GETITEMDATA, i, 0);
            if(targetID == id)
                return i;
        }
    }

    return INVALID;
}

UINT OvrFindFontName(ConfigOverlaySourceInfo *configInfo, HWND hwndFontList, CTSTR lpFontFace)
{
    return configInfo->fontNames.FindValueIndexI(lpFontFace);
}

CTSTR OvrGetFontFace(ConfigOverlaySourceInfo *configInfo, HWND hwndFontList)
{
    UINT id = (UINT)SendMessage(hwndFontList, CB_GETCURSEL, 0, 0);
    if(id == CB_ERR)
        return NULL;

    UINT actualID = (UINT)SendMessage(hwndFontList, CB_GETITEMDATA, id, 0);
    return configInfo->fontFaces[actualID];
}

static DWORD SelectColour(HWND hwnd, DWORD curCol)
{
	static CHOOSECOLOR cc;
	zero(&cc, sizeof(cc));

	static COLORREF crCustColors[16];
	static DWORD customColors[16] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
									 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
									 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
									 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};

	cc.lStructSize = sizeof(cc);
	cc.rgbResult = curCol;
	cc.lpCustColors = customColors;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;
	cc.hwndOwner = hwnd;

	if (ChooseColor(&cc) == TRUE)
	{
		return cc.rgbResult;// | 0xFFFFFFFF;
	}
	else
	{
		return -1 | 0xFFFFFFFF;
	}
}

INT_PTR CALLBACK ConfigureOverlayProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool bInitializedDialog = false;

    switch(message)
    {
        case WM_INITDIALOG:
            {
                ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)lParam;
                SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)configInfo);
                LocalizeWindow(hwnd);

                XElement *data = configInfo->data;

				fcolour = data->GetInt(TEXT("color"), 0xFFFFFFFF);
				bgcolour = data->GetInt(TEXT("backgroundColor"), 0xFF000000);
				olcolour = data->GetInt(TEXT("outlineColor"), 0xFF000000);

                //-----------------------------------------
				SendMessage(GetDlgItem(hwnd, IDC_NAME), UDM_SETRANGE32, 1, 128);
				SendMessage(GetDlgItem(hwnd, IDC_NAME), UDM_SETPOS32, 0, data->GetInt(TEXT("nameNumber")));
				SendMessage(GetDlgItem(hwnd, IDC_HIDEOWNNAME), BM_SETCHECK, data->GetInt(TEXT("bHideName"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hwnd, IDC_RIGHTSYMBOL), BM_SETCHECK, data->GetInt(TEXT("bRightSymbol"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
				//-----------------------------------------

                HDC hDCtest = GetDC(hwnd);

                LOGFONT lf;
                zero(&lf, sizeof(lf));
                EnumFontFamiliesEx(hDCtest, &lf, (FONTENUMPROC)OvrFontEnumProcThingy, (LPARAM)configInfo, 0);

                HWND hwndFonts = GetDlgItem(hwnd, IDC_FONT);
                for(UINT i=0; i<configInfo->fontNames.Num(); i++)
                {
                    int id = (int)SendMessage(hwndFonts, CB_ADDSTRING, 0, (LPARAM)configInfo->fontNames[i].Array());
                    SendMessage(hwndFonts, CB_SETITEMDATA, id, (LPARAM)i);
                }

                CTSTR lpFont = data->GetString(TEXT("font"));
                UINT id = OvrFindFontFace(configInfo, hwndFonts, lpFont);
                if(id == INVALID)
                    id = (UINT)SendMessage(hwndFonts, CB_FINDSTRINGEXACT, -1, (LPARAM)TEXT("Arial"));

                SendMessage(hwndFonts, CB_SETCURSEL, id, 0);

                //-----------------------------------------

                SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_SETRANGE32, 5, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_SETPOS32, 0, data->GetInt(TEXT("fontSize"), 48));

                //-----------------------------------------

                SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("textOpacity"), 100));

                SendMessage(GetDlgItem(hwnd, IDC_BOLD), BM_SETCHECK, data->GetInt(TEXT("bold"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_ITALIC), BM_SETCHECK, data->GetInt(TEXT("italic"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_UNDERLINE), BM_SETCHECK, data->GetInt(TEXT("underline"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);

                BOOL bUsePointFilter = data->GetInt(TEXT("pointFiltering"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_POINTFILTERING), BM_SETCHECK, bUsePointFilter ? BST_CHECKED : BST_UNCHECKED, 0);

                //-----------------------------------------

                SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("backgroundOpacity"), 0));

                //-----------------------------------------

                bool bChecked = data->GetInt(TEXT("useOutline"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_USEOUTLINE), BM_SETCHECK, bChecked ? BST_CHECKED : BST_UNCHECKED, 0);

                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OLCOLOUR), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), bChecked);

                SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_SETRANGE32, 1, 20);
                SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_SETPOS32, 0, data->GetInt(TEXT("outlineSize"), 2));

                SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("outlineOpacity"), 100));

                //-----------------------------------------

                bChecked = data->GetInt(TEXT("useTextExtents"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_USETEXTEXTENTS), BM_SETCHECK, bChecked ? BST_CHECKED : BST_UNCHECKED, 0);
                ConfigureOverlayProc(hwnd, WM_COMMAND, MAKEWPARAM(IDC_USETEXTEXTENTS, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_USETEXTEXTENTS));

                EnableWindow(GetDlgItem(hwnd, IDC_EXTENTWIDTH_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_EXTENTHEIGHT_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_EXTENTWIDTH), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_WRAP), bChecked);

                bool bVertical = data->GetInt(TEXT("vertical"), 0) != 0;

                SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_SETRANGE32, 32, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_SETRANGE32, 32, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_SETPOS32, 0, data->GetInt(TEXT("extentWidth"),  100));
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_SETPOS32, 0, data->GetInt(TEXT("extentHeight"), 100));

                bool bWrap = data->GetInt(TEXT("wrap"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_WRAP), BM_SETCHECK, bWrap ? BST_CHECKED : BST_UNCHECKED, 0);

                bool bScrollMode = data->GetInt(TEXT("scrollMode"), 0) != 0;

                EnableWindow(GetDlgItem(hwnd, IDC_ALIGN), bChecked && bWrap);
                
                HWND hwndAlign = GetDlgItem(hwnd, IDC_ALIGN);
                SendMessage(hwndAlign, CB_ADDSTRING, 0, LPARAM(TEXT("Left")));
                SendMessage(hwndAlign, CB_ADDSTRING, 0, LPARAM(TEXT("Center")));
                SendMessage(hwndAlign, CB_ADDSTRING, 0, LPARAM(TEXT("Right")));

                int align = data->GetInt(TEXT("align"), 0);
                ClampVal(align, 0, 2);
                SendMessage(hwndAlign, CB_SETCURSEL, align, 0);

                //-----------------------------------------

                BOOL bUseFile = data->GetInt(TEXT("mode"), 0) == 1;

                bInitializedDialog = true;

                return TRUE;
            }

        case WM_DESTROY:
            bInitializedDialog = false;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_FONT:
                    if(bInitializedDialog)
                    {
                        if(HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE)
                        {
                            ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                            if(!configInfo) break;

                            String strFont;
                            if(HIWORD(wParam) == CBN_SELCHANGE)
                                strFont = OvrGetFontFace(configInfo, (HWND)lParam);
                            else
                            {
                                UINT id = OvrFindFontName(configInfo, (HWND)lParam, GetEditText((HWND)lParam));
                                if(id != INVALID)
                                    strFont = configInfo->fontFaces[id];
                            }

                            ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                            if(source && strFont.IsValid())
                                source->SetString(TEXT("font"), strFont);
                        }
                    }
                    break;
				
				case IDC_FCOLOUR:
					if(bInitializedDialog)
					{
						DWORD ofcolour = fcolour;
						fcolour = SelectColour(hwnd, ofcolour | 0xFF000000);

						ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
							source->SetInt(TEXT("color"), fcolour | 0xFF000000); break;
                        }
                    }
                    break;
				case IDC_BGCOLOUR:
					if(bInitializedDialog)
					{
						DWORD obgcolour = bgcolour;
						bgcolour = SelectColour(hwnd, obgcolour | 0xFF000000);

						ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
						{
							source->SetInt(TEXT("backgroundColor"), bgcolour | 0xFF000000); break;
						}
                    }
                    break;
				case IDC_OLCOLOUR:
					if(bInitializedDialog)
					{
						DWORD oolcolour = olcolour | 0xFF000000;
						olcolour = SelectColour(hwnd, oolcolour);

						ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
						{
							source->SetInt(TEXT("outlineColor"), olcolour | 0xFF000000); break;
						}
                    }
                    break;

                case IDC_TEXTSIZE_EDIT:
                case IDC_EXTENTWIDTH_EDIT:
                case IDC_EXTENTHEIGHT_EDIT:
                case IDC_BACKGROUNDOPACITY_EDIT:
                case IDC_TEXTOPACITY_EDIT:
                case IDC_OUTLINEOPACITY_EDIT:
                case IDC_OUTLINETHICKNESS_EDIT:
				case IDC_NAME_EDIT:
                    if(HIWORD(wParam) == EN_CHANGE && bInitializedDialog)
                    {
                        int val = (int)SendMessage(GetWindow((HWND)lParam, GW_HWNDNEXT), UDM_GETPOS32, 0, 0);

                        ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;

                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                case IDC_TEXTSIZE_EDIT:             source->SetInt(TEXT("fontSize"), val); break;
                                case IDC_EXTENTWIDTH_EDIT:          source->SetInt(TEXT("extentWidth"), val); break;
                                case IDC_EXTENTHEIGHT_EDIT:         source->SetInt(TEXT("extentHeight"), val); break;
                                case IDC_TEXTOPACITY_EDIT:          source->SetInt(TEXT("textOpacity"), val); break;
                                case IDC_OUTLINEOPACITY_EDIT:       source->SetInt(TEXT("outlineOpacity"), val); break;
                                case IDC_BACKGROUNDOPACITY_EDIT:    source->SetInt(TEXT("backgroundOpacity"), val); break;
                                case IDC_OUTLINETHICKNESS_EDIT:     source->SetFloat(TEXT("outlineSize"), (float)val); break;
								case IDC_NAME_EDIT:	
									source->SetInt(TEXT("nameNumber"), val);
									iname = val;
									break;
                            }
                        }
                    }
                    break;

                case IDC_BOLD:
                case IDC_ITALIC:
                case IDC_UNDERLINE:
                case IDC_WRAP:
                case IDC_USEOUTLINE:
                case IDC_USETEXTEXTENTS:
				case IDC_HIDEOWNNAME:
				case IDC_RIGHTSYMBOL:
                    if(HIWORD(wParam) == BN_CLICKED && bInitializedDialog)
                    {
                        BOOL bChecked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;

                        ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                case IDC_BOLD:              source->SetInt(TEXT("bold"), bChecked); break;
                                case IDC_ITALIC:            source->SetInt(TEXT("italic"), bChecked); break;
                                case IDC_UNDERLINE:         source->SetInt(TEXT("underline"), bChecked); break;
                                case IDC_WRAP:              source->SetInt(TEXT("wrap"), bChecked); break;
                                case IDC_USEOUTLINE:        source->SetInt(TEXT("useOutline"), bChecked); break;
                                case IDC_USETEXTEXTENTS:    source->SetInt(TEXT("useTextExtents"), bChecked); break;
								case IDC_HIDEOWNNAME:
									source->SetInt(TEXT("bHideName"), bChecked);
									bname = bChecked;
									break;
								case IDC_RIGHTSYMBOL:
									source->SetInt(TEXT("bRightSymbol"), bChecked);
									bright = bChecked;
									break;
                            }
                        }

                        if(LOWORD(wParam) == IDC_WRAP)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_ALIGN), bChecked);
                        }
                        else if(LOWORD(wParam) == IDC_USETEXTEXTENTS)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_EXTENTWIDTH_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_EXTENTHEIGHT_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_EXTENTWIDTH), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_WRAP), bChecked);

                            bool bWrap = SendMessage(GetDlgItem(hwnd, IDC_WRAP), BM_GETCHECK, 0, 0) == BST_CHECKED;
                            
                            //EnableWindow(GetDlgItem(hwnd, IDC_ALIGN), bChecked && bWrap);
                        }
                        else if(LOWORD(wParam) == IDC_USEOUTLINE)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OLCOLOUR), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), bChecked);
                        }
                    }
                    break;

                case IDC_ALIGN:
                    if(HIWORD(wParam) == CBN_SELCHANGE && bInitializedDialog)
                    {
                        int align = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                        if(align == CB_ERR)
                            break;

                        ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                            source->SetInt(TEXT("align"), align);
                    }
                    break;

                case IDOK:
                    {
                        ConfigOverlaySourceInfo *configInfo = (ConfigOverlaySourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        XElement *data = configInfo->data;

                        BOOL bUseTextExtents = SendMessage(GetDlgItem(hwnd, IDC_USETEXTEXTENTS), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bUseOutline = SendMessage(GetDlgItem(hwnd, IDC_USEOUTLINE), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        float outlineSize = (float)SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_GETPOS32, 0, 0);

                        UINT extentWidth  = (UINT)SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_GETPOS32, 0, 0);
                        UINT extentHeight = (UINT)SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_GETPOS32, 0, 0);

                        String strFont = OvrGetFontFace(configInfo, GetDlgItem(hwnd, IDC_FONT));
                        UINT fontSize = (UINT)SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_GETPOS32, 0, 0);

                        BOOL bBold = SendMessage(GetDlgItem(hwnd, IDC_BOLD), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bItalic = SendMessage(GetDlgItem(hwnd, IDC_ITALIC), BM_GETCHECK, 0, 0) == BST_CHECKED;

						iname = SendMessage(GetDlgItem(hwnd, IDC_NAME), UDM_GETPOS32, 0, 0);
						data->SetInt(TEXT("nameNumber"), iname);
						bname = SendMessage(GetDlgItem(hwnd, IDC_HIDEOWNNAME), BM_GETCHECK, 0, 0) == BST_CHECKED;
						data->SetInt(TEXT("bHideName"), bname);
						bright = SendMessage(GetDlgItem(hwnd, IDC_RIGHTSYMBOL), BM_GETCHECK, 0, 0) == BST_CHECKED;
						data->SetInt(TEXT("bRightSymbol"), bright);

                        BOOL pointFiltering = SendMessage(GetDlgItem(hwnd, IDC_POINTFILTERING), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        String strFontDisplayName = GetEditText(GetDlgItem(hwnd, IDC_FONT));
                        if(strFont.IsEmpty())
                        {
                            UINT id = OvrFindFontName(configInfo, GetDlgItem(hwnd, IDC_FONT), strFontDisplayName);
                            if(id != INVALID)
                                strFont = configInfo->fontFaces[id];
                        }

                        if(strFont.IsEmpty())
                        {
                            String strError = Str("Sources.TextSource.FontNotFound");
                            strError.FindReplace(TEXT("$1"), strFontDisplayName);
                            MessageBox(hwnd, strError, NULL, 0);
                            break;
                        }

                        if(bUseTextExtents)
                        {
                            configInfo->cx = float(extentWidth);
                            configInfo->cy = float(extentHeight);
                        }
                        else
                        {
                            String strOutputText;
                            XFile textFile;
                            textFile.ReadFileToString(strOutputText);

                            LOGFONT lf;
                            zero(&lf, sizeof(lf));
                            lf.lfHeight = fontSize;
                            lf.lfWeight = bBold ? FW_BOLD : FW_DONTCARE;
                            lf.lfItalic = bItalic;
                            lf.lfQuality = ANTIALIASED_QUALITY;
                            if(strFont.IsValid())
                                scpy_n(lf.lfFaceName, strFont, 31);
                            else
                                scpy_n(lf.lfFaceName, TEXT("Arial"), 31);

                            HDC hDC = CreateCompatibleDC(NULL);

                            Gdiplus::Font font(hDC, &lf);

                            if(!font.IsAvailable())
                            {
                                String strError = Str("Sources.TextSource.FontNotFound");
                                strError.FindReplace(TEXT("$1"), strFontDisplayName);
                                MessageBox(hwnd, strError, NULL, 0);
                                DeleteDC(hDC);
                                break;
                            }

                            {
                                Gdiplus::Graphics graphics(hDC);
                                Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());

                                UINT formatFlags;

                                formatFlags = Gdiplus::StringFormatFlagsNoFitBlackBox
                                            | Gdiplus::StringFormatFlagsMeasureTrailingSpaces;


                                format.SetFormatFlags(formatFlags);
                                format.SetTrimming(Gdiplus::StringTrimmingWord);

                                Gdiplus::RectF rcf;
                                graphics.MeasureString(strOutputText, -1, &font, Gdiplus::PointF(0.0f, 0.0f), &format, &rcf);

                                if(bUseOutline)
                                {
                                    rcf.Height += outlineSize;
                                    rcf.Width  += outlineSize;
                                }

                                if(rcf.Height<fontSize)
                                        rcf.Height = (float)fontSize;
                                
                                configInfo->cx = MAX(rcf.Width,  32.0f);
                                configInfo->cy = MAX(rcf.Height, 32.0f);
                            }

                            DeleteDC(hDC);
                        }

                        data->SetFloat(TEXT("baseSizeCX"), configInfo->cx);
                        data->SetFloat(TEXT("baseSizeCY"), configInfo->cy);

						DWORD nfcolour = REVERSE_COLOR(fcolour);
						DWORD nbgcolour = REVERSE_COLOR(bgcolour);
						DWORD nolcolour = REVERSE_COLOR(olcolour);

                        data->SetString(TEXT("font"), strFont);
                        data->SetInt(TEXT("color"), nfcolour);
                        data->SetInt(TEXT("fontSize"), fontSize);
                        data->SetInt(TEXT("textOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_GETPOS32, 0, 0));
                        data->SetInt(TEXT("bold"), bBold);
                        data->SetInt(TEXT("italic"), bItalic);
                        data->SetInt(TEXT("wrap"), SendMessage(GetDlgItem(hwnd, IDC_WRAP), BM_GETCHECK, 0, 0) == BST_CHECKED);
                        data->SetInt(TEXT("underline"), SendMessage(GetDlgItem(hwnd, IDC_UNDERLINE), BM_GETCHECK, 0, 0) == BST_CHECKED);
                        data->SetInt(TEXT("pointFiltering"), pointFiltering);

                        data->SetInt(TEXT("backgroundColor"), nbgcolour);
                        data->SetInt(TEXT("backgroundOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_GETPOS32, 0, 0));

                        data->SetInt(TEXT("useOutline"), bUseOutline);
                        data->SetInt(TEXT("outlineColor"), nolcolour);
                        data->SetFloat(TEXT("outlineSize"), outlineSize);
                        data->SetInt(TEXT("outlineOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_GETPOS32, 0, 0));

                        data->SetInt(TEXT("useTextExtents"), bUseTextExtents);
                        data->SetInt(TEXT("extentWidth"), extentWidth);
                        data->SetInt(TEXT("extentHeight"), extentHeight);
                        data->SetInt(TEXT("align"), (int)SendMessage(GetDlgItem(hwnd, IDC_ALIGN), CB_GETCURSEL, 0, 0));

						data->SetInt(TEXT("nameNumber"), iname);
						data->SetInt(TEXT("bHideName"), bname);
						data->SetInt(TEXT("bRightSymbol"), bright);
                    }

                case IDCANCEL:
                    if(LOWORD(wParam) == IDCANCEL)
                        OvrDoCancelStuff(hwnd);

                    EndDialog(hwnd, LOWORD(wParam));
            }
            break;

        case WM_CLOSE:
            OvrDoCancelStuff(hwnd);
            EndDialog(hwnd, IDCANCEL);
    }
    return 0;
}

bool STDCALL ConfigureOverlaySource(XElement *element, bool bCreating)
{
    if(!element)
    {
        AppWarning(TEXT("ConfigureOverlaySource: NULL element"));
        return false;
    }

    XElement *data = element->GetElement(TEXT("data"));
    if(!data)
        data = element->CreateElement(TEXT("data"));

    ConfigOverlaySourceInfo configInfo;
    configInfo.lpName = element->GetName();
    configInfo.data = data;

    if(DialogBoxParam(GetHinstance(), MAKEINTRESOURCE(IDD_CONFIGUREOVERLAYSOURCE), OBSGetMainWindow(), ConfigureOverlayProc, (LPARAM)&configInfo) == IDOK)
    {
        element->SetFloat(TEXT("cx"), configInfo.cx);
        element->SetFloat(TEXT("cy"), configInfo.cy);

        return true;
    }
    return false;
}

int GetNumberOfNames()
{
	return iname;
}

bool GetHideSelf()
{
	return bname;
}

bool GetRightOfSymbol()
{
	return bright;
}