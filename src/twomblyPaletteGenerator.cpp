#include "twomblyPaletteGenerator.h"

twomblyPaletteGenerator::twomblyPaletteGenerator()
{
    
}

void twomblyPaletteGenerator::setup(string &fileName)
{
    ofImage source;
    source.loadImage(fileName);
    int imgWidth = source.getWidth();
    int imgHeight = source.getHeight();
    for (int x = 0; x < imgWidth; x += SAMPLE_RESOLUTION)
    {
        for (int y = 0; y < imgHeight; y += SAMPLE_RESOLUTION)
        {
            m_palette.push_back(ofColor(source.getPixelsRef().getColor(x, y), SAMPLE_ALPHA));
        }
    }
    refinePalette();
}

void twomblyPaletteGenerator::draw()
{
    int numPerRow = 10;
    int rowNum = 1;
    for (int i = 0; i < m_palette.size(); i++)
    {
        ofSetColor(m_palette[i]);
        ofRect((i % numPerRow) * 20, rowNum * 20, 20, 20);
        if (i % numPerRow == 0) rowNum++;
    }
}

vector<ofColor>& twomblyPaletteGenerator::getPalette()
{
    return m_palette;
}

void twomblyPaletteGenerator::refinePalette()
{
    for (int i = 0; i < m_palette.size(); i++)
    {
        ofColor currentColor = m_palette[i];
        for (int j = i + 1; j < m_palette.size() - 1; j++)
        {
            float difference = 0.0f;
            difference += abs(currentColor.r - m_palette[j].r);
            difference += abs(currentColor.g - m_palette[j].g);
            difference += abs(currentColor.b - m_palette[j].b);
            if (difference < MIN_SAMPLE_DIFFERENCE) m_palette.erase(m_palette.begin() + j);
        }
    }
}