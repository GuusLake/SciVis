#include "qscreen.h"
#include "visualization.h"

#include "mainwindow.h"


// Generate all necessary VAOs, VBOs, EBOs and texture names
void Visualization::opengl_generateObjects()
{
    glGenVertexArrays(1, &m_vaoScalarData);
    glGenBuffers(1, &m_vboScalarPoints);
    glGenBuffers(1, &m_vboScalarData);
    glGenBuffers(1, &m_eboScalarData);
    glGenTextures(1, &m_scalarDataTextureLocation);

    glGenVertexArrays(1, &m_vaoGlyphs);
    glGenBuffers(1, &m_vboGlyphs);
    glGenBuffers(1, &m_eboGlyphs);
    glGenBuffers(1, &m_vboModelTransformationMatricesGlyphs);
    glGenBuffers(1, &m_vboValuesGlyphs);
    glGenTextures(1, &m_vectorDataTextureLocation);

    glGenVertexArrays(1, &m_vaoIsolines);
    glGenBuffers(1, &m_vboIsolines);
    glGenTextures(1, &m_isolinesTextureLocation);

    glGenVertexArrays(1, &m_vaoHeightplot);
    glGenBuffers(1, &m_vboHeightplotPoints);
    glGenBuffers(1, &m_vboHeightplotScalarValues);
    glGenBuffers(1, &m_vboHeightplotHeight);
    glGenBuffers(1, &m_vboHeightplotNormals);
    glGenBuffers(1, &m_eboHeightplot);

    glGenVertexArrays(1, &m_vaoLic);
    glGenBuffers(1, &m_vboLic);
    glGenTextures(1, &m_licTextureLocation);

    glGenVertexArrays(1, &m_vaoVolumeRendering);
    glGenBuffers(1, &m_vboVolumeRendering);
    glGenBuffers(1, &m_eboVolumeRendering);
}

void Visualization::opengl_createShaderPrograms()
{
    opengl_createShaderProgramScalarDataScaleTexture();
    opengl_createShaderProgramScalarDataScaleCustomColorMap();
    opengl_createShaderProgramScalarDataClampTexture();
    opengl_createShaderProgramScalarDataClampCustomColorMap();
    opengl_createShaderProgramColorMapInstanced();
    opengl_createShaderProgramIsolines();
    opengl_createShaderProgramHeightplotScale();
    opengl_createShaderProgramHeightplotClamp();
    opengl_createShaderProgramLic();
    opengl_createShaderProgramVolumeRendering();
}

void Visualization::opengl_setupAllBuffers()
{
    opengl_setupScalarData();
    opengl_setupGlyphs();
    opengl_setupIsolines();
    opengl_setupHeightplot();
    opengl_setupLic();
    opengl_setupVolumeRendering();
}

void Visualization::opengl_deleteObjects()
{
    glDeleteVertexArrays(1, &m_vaoScalarData);
    glDeleteBuffers(1, &m_vboScalarPoints);
    glDeleteBuffers(1, &m_vboScalarData);
    glDeleteBuffers(1, &m_eboScalarData);

    glDeleteVertexArrays(1, &m_vaoGlyphs);
    glDeleteBuffers(1, &m_vboGlyphs);
    glDeleteBuffers(1, &m_eboGlyphs);
    glDeleteBuffers(1, &m_vboModelTransformationMatricesGlyphs);
    glDeleteBuffers(1, &m_vboValuesGlyphs);

    glDeleteVertexArrays(1, &m_vaoIsolines);
    glDeleteBuffers(1, &m_vboIsolines);

    glDeleteVertexArrays(1, &m_vaoHeightplot);
    glDeleteBuffers(1, &m_vboHeightplotPoints);
    glDeleteBuffers(1, &m_vboHeightplotHeight);
    glDeleteBuffers(1, &m_vboHeightplotScalarValues);
    glDeleteBuffers(1, &m_vboHeightplotNormals);
    glDeleteBuffers(1, &m_eboHeightplot);

    glDeleteBuffers(1, &m_vaoLic);
    glDeleteBuffers(1, &m_vboLic);
    glDeleteTextures(1, &m_licTextureLocation);

    glDeleteTextures(1, &m_scalarDataTextureLocation);
    glDeleteTextures(1, &m_vectorDataTextureLocation);

    glDeleteVertexArrays(1, &m_vaoVolumeRendering);
    glDeleteBuffers(1, &m_vboVolumeRendering);
    glDeleteBuffers(1, &m_eboVolumeRendering);
}

void Visualization::opengl_loadScalarDataTexture(std::vector<Color> const &colorMap)
{
    glBindTexture(GL_TEXTURE_1D, m_scalarDataTextureLocation);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage1D(GL_TEXTURE_1D,
                 0,
                 GL_RGB32F,
                 static_cast<GLint>(colorMap.size()),
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 colorMap.data());
}

void Visualization::opengl_setupScalarData()
{
    glBindVertexArray(m_vaoScalarData);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboScalarPoints);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * 2U * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0U);
    glVertexAttribPointer(0U, 2, GL_FLOAT, GL_FALSE, 0U, reinterpret_cast<GLvoid*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, m_vboScalarData);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1U);
    glVertexAttribPointer(1U, 1, GL_FLOAT, GL_FALSE, 0U, reinterpret_cast<GLvoid*>(0));

    size_t const numberOfTriangleStripIndices = (m_DIM - 1U) * (2U * m_DIM + 2U) - 2U;
    m_indices.reserve(numberOfTriangleStripIndices);

    for (unsigned short stripIdx = 0U; stripIdx < (m_DIM * (m_DIM - 1U)); stripIdx += m_DIM)
    {
        unsigned short lastUsedIdx;
        for (unsigned short idx = stripIdx; idx < (stripIdx + m_DIM); ++idx)
        {
            m_indices.push_back(idx);

            lastUsedIdx = static_cast<unsigned short>(idx + m_DIM);
            m_indices.push_back(lastUsedIdx);
        }

        // Add degenerate vertices to start rendering the next strip without requiring a new (expensive) draw call.
        // Note: there's no special case for the last triangle, so a couple of redundant indices are added.
        m_indices.push_back(lastUsedIdx); // Repeat last added vertex.
        m_indices.push_back(static_cast<unsigned short>(stripIdx + m_DIM)); // Add first vertex of next strip, so that it will appear twice.
    }

    // No primitive restart, so the last (degenerated) triangles can be removed.
    m_indices.erase(m_indices.end() - 2U, m_indices.end());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboScalarData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned short)),
                 m_indices.data(),
                 GL_STATIC_DRAW);
}

void Visualization::opengl_setupIsolines()
{
    glBindVertexArray(m_vaoIsolines);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIsolines);

    // Set vertex coordinates to location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<GLvoid*>(0));

    // Set height to location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<GLvoid*>(2 * sizeof(float)));
}

void Visualization::opengl_setupGlyphs()
{
    opengl_bufferSingleGlyph();
    opengl_setupGlyphsPerInstanceData();
}

void Visualization::opengl_bufferSingleGlyph()
{
    std::pair<std::vector<QVector3D>, std::vector<unsigned short>> singleGlyph;

    switch (m_currentGlyphType)
    {
        case Glyph::GlyphType::Hedgehog:
            singleGlyph = Glyph::hedgehog();
        break;

        case Glyph::GlyphType::Triangle:
            singleGlyph = Glyph::triangle(0.3F);
        break;

        case Glyph::GlyphType::Arrow2D:
            singleGlyph = Glyph::arrow2D(0.1F, 0.5F, 0.3F);
        break;

        case Glyph::GlyphType::Cone:
            singleGlyph = Glyph::cone(0.3F, 10);
        break;

        case Glyph::GlyphType::Arrow3D:
            singleGlyph = Glyph::arrow3D(0.1F, 0.5F, 0.3F, 10U);
        break;
    }

    auto const glyphVertices = singleGlyph.first;
    auto const glyphIndices = singleGlyph.second;
    m_glyphIndicesSize = glyphIndices.size();

    glBindVertexArray(m_vaoGlyphs);

    // Buffer a single glyph.
    glBindBuffer(GL_ARRAY_BUFFER, m_vboGlyphs);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(glyphVertices.size() * 3 * sizeof(float)),
                 glyphVertices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));

    // Buffer indices for the glyph.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboGlyphs);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_glyphIndicesSize * sizeof(unsigned short)),
                 glyphIndices.data(),
                 GL_STATIC_DRAW);
}

void Visualization::opengl_setupGlyphsPerInstanceData()
{
    // Buffering section starts here.
    glBindVertexArray(m_vaoGlyphs);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboValuesGlyphs);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_numberOfGlyphsX * m_numberOfGlyphsY * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);

    // Buffer values.
    static bool firstRun = true;
    if (firstRun)
    {
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);
    }

    // Buffer model transformation matrices.
    glBindBuffer(GL_ARRAY_BUFFER, m_vboModelTransformationMatricesGlyphs);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_numberOfGlyphsX * m_numberOfGlyphsY * 16 * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);

    // A location can maximally hold 4 values, so for a 4x4 matrix,
    // 4 attribute pointers need to be defined.
    if (firstRun)
    {
        firstRun = false;
        for (unsigned int columnIdx = 0; columnIdx < 4; ++columnIdx)
        {
            glVertexAttribPointer(2 + columnIdx,
                                  4,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  16 * sizeof(float),
                                  reinterpret_cast<GLvoid*>(4 * sizeof(float) * columnIdx));
            glEnableVertexAttribArray(2 + columnIdx);
            glVertexAttribDivisor(2 + columnIdx, 1);
        }
    }
}

void Visualization::opengl_setupHeightplot()
{
    glBindVertexArray(m_vaoHeightplot);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotPoints);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * 2 * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotHeight);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotScalarValues);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotNormals);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_DIM * m_DIM * 3 * sizeof(float)),
                 static_cast<GLvoid*>(nullptr),
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboHeightplot);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned short)),
                 m_indices.data(),
                 GL_STATIC_DRAW);
}

void Visualization::opengl_setupLic()
{
    m_licObject.resetTexture();

    glBindVertexArray(m_vaoLic);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboLic);

    glEnableVertexAttribArray(0U);
    glVertexAttribPointer(0U, 2, GL_FLOAT, GL_FALSE, 2U * sizeof(QVector2D), reinterpret_cast<GLvoid*>(0));

    glEnableVertexAttribArray(1U);
    glVertexAttribPointer(1U, 2, GL_FLOAT, GL_FALSE, 2U * sizeof(QVector2D), reinterpret_cast<GLvoid*>(2U * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, m_vboLic);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(8U * sizeof(QVector2D)),
                 nullptr,
                 GL_STATIC_DRAW);
}

// Render a window-sized quad.
void Visualization::opengl_setupVolumeRendering()
{
    std::array<QVector2D, 4U> const vertices{QVector2D{-1.0F,  1.0F},  // Top left.
                                             QVector2D{-1.0F, -1.0F},  // Bottom left.
                                             QVector2D{ 1.0F,  1.0F},  // Top right.
                                             QVector2D{ 1.0F, -1.0F}}; // Bottom right.
    std::array<unsigned short, 6U> const indices{0U, 1U, 2U,
                                                 1U, 3U, 2U};

    // Buffer data
    glBindVertexArray(m_vaoVolumeRendering);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboVolumeRendering);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(QVector2D)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // Buffer indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboVolumeRendering);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned short)),
                 indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), reinterpret_cast<GLvoid*>(0));
}

void Visualization::opengl_createShaderProgramScalarDataScaleTexture()
{
    m_shaderProgramScalarDataScaleTexture.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/scalarData_scale.vert");
    m_shaderProgramScalarDataScaleTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/scalarData_texture.frag");
    m_shaderProgramScalarDataScaleTexture.link();

    m_uniformLocationScalarDataScaleTexture_projection = m_shaderProgramScalarDataScaleTexture.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationScalarDataScaleTexture_projection != -1);
    m_uniformLocationScalarDataScaleTexture_texture = m_shaderProgramScalarDataScaleTexture.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationScalarDataScaleTexture_texture != -1);

    m_uniformLocationScalarDataScaleTexture_rangeMin = m_shaderProgramScalarDataScaleTexture.uniformLocation("rangeMin");
    Q_ASSERT(m_uniformLocationScalarDataScaleTexture_rangeMin != -1);
    m_uniformLocationScalarDataScaleTexture_rangeMax = m_shaderProgramScalarDataScaleTexture.uniformLocation("rangeMax");
    Q_ASSERT(m_uniformLocationScalarDataScaleTexture_rangeMax != -1);
    m_uniformLocationScalarDataScaleTexture_transferK = m_shaderProgramScalarDataScaleTexture.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationScalarDataScaleTexture_transferK != -1);

    qDebug() << "m_shaderProgramScalarDataScaleTexture initialized.";
}

void Visualization::opengl_createShaderProgramScalarDataScaleCustomColorMap()
{
    m_shaderProgramScalarDataScaleCustomColorMap.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/scalarData_scale.vert");
    m_shaderProgramScalarDataScaleCustomColorMap.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/scalarData_customcolormap.frag");
    m_shaderProgramScalarDataScaleCustomColorMap.link();

    m_uniformLocationScalarDataScaleCustomColorMap_projection = m_shaderProgramScalarDataScaleCustomColorMap.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationScalarDataScaleCustomColorMap_projection != -1);

    m_uniformLocationScalarDataScaleCustomColorMap_colorMapColors = m_shaderProgramScalarDataScaleCustomColorMap.uniformLocation("colorMapColors");
    Q_ASSERT(m_uniformLocationScalarDataScaleCustomColorMap_colorMapColors != -1);

    m_uniformLocationScalarDataScaleCustomColorMap_rangeMin = m_shaderProgramScalarDataScaleCustomColorMap.uniformLocation("rangeMin");
    Q_ASSERT(m_uniformLocationScalarDataScaleCustomColorMap_rangeMin != -1);
    m_uniformLocationScalarDataScaleCustomColorMap_rangeMax = m_shaderProgramScalarDataScaleCustomColorMap.uniformLocation("rangeMax");
    Q_ASSERT(m_uniformLocationScalarDataScaleCustomColorMap_rangeMax != -1);
    m_uniformLocationScalarDataScaleCustomColorMap_transferK = m_shaderProgramScalarDataScaleCustomColorMap.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationScalarDataScaleCustomColorMap_transferK != -1);

    qDebug() << "m_shaderProgramScalarDataScaleCustomColorMap initialized.";
}

void Visualization::opengl_createShaderProgramScalarDataClampTexture()
{
    m_shaderProgramScalarDataClampTexture.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/scalarData_clamp.vert");
    m_shaderProgramScalarDataClampTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/scalarData_texture.frag");
    m_shaderProgramScalarDataClampTexture.link();

    m_uniformLocationScalarDataClampTexture_projection = m_shaderProgramScalarDataClampTexture.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationScalarDataClampTexture_projection != -1);
    m_uniformLocationScalarDataClampTexture_texture = m_shaderProgramScalarDataClampTexture.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationScalarDataClampTexture_texture != -1);

    m_uniformLocationScalarDataClampTexture_clampMin = m_shaderProgramScalarDataClampTexture.uniformLocation("clampMin");
    Q_ASSERT(m_uniformLocationScalarDataClampTexture_clampMin != -1);
    m_uniformLocationScalarDataClampTexture_clampMax = m_shaderProgramScalarDataClampTexture.uniformLocation("clampMax");
    Q_ASSERT(m_uniformLocationScalarDataClampTexture_clampMax != -1);
    m_uniformLocationScalarDataClampTexture_transferK = m_shaderProgramScalarDataClampTexture.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationScalarDataClampTexture_transferK != -1);

    qDebug() << "m_shaderProgramScalarDataClampTexture initialized.";
}

void Visualization::opengl_createShaderProgramScalarDataClampCustomColorMap()
{
    m_shaderProgramScalarDataClampCustomColorMap.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/scalarData_clamp.vert");
    m_shaderProgramScalarDataClampCustomColorMap.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/scalarData_customcolormap.frag");
    m_shaderProgramScalarDataClampCustomColorMap.link();

    m_uniformLocationScalarDataClampCustomColorMap_projection = m_shaderProgramScalarDataClampCustomColorMap.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationScalarDataClampCustomColorMap_projection != -1);

    m_uniformLocationScalarDataClampCustomColorMap_colorMapColors = m_shaderProgramScalarDataClampCustomColorMap.uniformLocation("colorMapColors");
    Q_ASSERT(m_uniformLocationScalarDataClampCustomColorMap_colorMapColors != -1);

    m_uniformLocationScalarDataClampCustomColorMap_clampMin = m_shaderProgramScalarDataClampCustomColorMap.uniformLocation("clampMin");
    Q_ASSERT(m_uniformLocationScalarDataClampCustomColorMap_clampMin != -1);
    m_uniformLocationScalarDataClampCustomColorMap_clampMax = m_shaderProgramScalarDataClampCustomColorMap.uniformLocation("clampMax");
    Q_ASSERT(m_uniformLocationScalarDataClampCustomColorMap_clampMax != -1);
    m_uniformLocationScalarDataClampCustomColorMap_transferK = m_shaderProgramScalarDataClampCustomColorMap.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationScalarDataClampCustomColorMap_transferK != -1);

    qDebug() << "m_shaderProgramScalarDataClampCustomColorMap initialized.";
}

void Visualization::opengl_createShaderProgramColorMapInstanced()
{
    m_shaderProgramVectorData.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/glyphsshading.vert");
    m_shaderProgramVectorData.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/glyphsshading.frag");
    m_shaderProgramVectorData.link();

    m_uniformLocationProjectionColorMapInstanced = m_shaderProgramVectorData.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationProjectionColorMapInstanced != -1);
    m_uniformLocationTextureColorMapInstanced = m_shaderProgramVectorData.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationTextureColorMapInstanced != -1);

    qDebug() << "m_shaderProgramVectorData initialized.";
}

void Visualization::opengl_createShaderProgramIsolines()
{
    m_shaderProgramIsolines.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/isolines.vert");
    m_shaderProgramIsolines.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/isolines.frag");
    m_shaderProgramIsolines.link();

    m_uniformLocationIsolines_projection = m_shaderProgramIsolines.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationIsolines_projection != -1);
    m_uniformLocationIsolines_color = m_shaderProgramIsolines.uniformLocation("isolineColor");
    Q_ASSERT(m_uniformLocationIsolines_color != -1);

    qDebug() << "m_shaderProgramIsolines initialized.";
}

void Visualization::opengl_createShaderProgramHeightplotScale()
{
    m_shaderProgramHeightplotScale.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/heightplot_scale.vert");
    m_shaderProgramHeightplotScale.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/heightplot.frag");
    m_shaderProgramHeightplotScale.link();

    m_uniformLocationHeightplotScale_rangeMin = m_shaderProgramHeightplotScale.uniformLocation("rangeMin");
    Q_ASSERT(m_uniformLocationHeightplotScale_rangeMin != -1);
    m_uniformLocationHeightplotScale_rangeMax = m_shaderProgramHeightplotScale.uniformLocation("rangeMax");
    Q_ASSERT(m_uniformLocationHeightplotScale_rangeMax != -1);
    m_uniformLocationHeightplotScale_transferK = m_shaderProgramHeightplotScale.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationHeightplotScale_transferK != -1);

    m_uniformLocationHeightplotScale_projection = m_shaderProgramHeightplotScale.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationHeightplotScale_projection != -1);
    m_uniformLocationHeightplotScale_view = m_shaderProgramHeightplotScale.uniformLocation("viewTransform");
    Q_ASSERT(m_uniformLocationHeightplotScale_view != -1);
    m_uniformLocationHeightplotScale_normal = m_shaderProgramHeightplotScale.uniformLocation("normalTransform");
    Q_ASSERT(m_uniformLocationHeightplotScale_normal != -1);

    m_uniformLocationHeightplotScale_material = m_shaderProgramHeightplotScale.uniformLocation("material");
    Q_ASSERT(m_uniformLocationHeightplotScale_material != -1);
    m_uniformLocationHeightplotScale_light = m_shaderProgramHeightplotScale.uniformLocation("lightPosition");
    Q_ASSERT(m_uniformLocationHeightplotScale_light != -1);

    m_uniformLocationHeightplotScale_texture = m_shaderProgramHeightplotScale.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationHeightplotScale_texture != -1);

    qDebug() << "m_shaderProgramHeightplotScale initialized.";
}

void Visualization::opengl_createShaderProgramHeightplotClamp()
{
    m_shaderProgramHeightplotClamp.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/heightplot_clamp.vert");
    m_shaderProgramHeightplotClamp.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/heightplot.frag");
    m_shaderProgramHeightplotClamp.link();

    m_uniformLocationHeightplotClamp_clampMin = m_shaderProgramHeightplotClamp.uniformLocation("clampMin");
    Q_ASSERT(m_uniformLocationHeightplotClamp_clampMin != -1);
    m_uniformLocationHeightplotClamp_clampMax = m_shaderProgramHeightplotClamp.uniformLocation("clampMax");
    Q_ASSERT(m_uniformLocationHeightplotClamp_clampMax != -1);
    m_uniformLocationHeightplotClamp_transferK = m_shaderProgramHeightplotClamp.uniformLocation("transferK");
    Q_ASSERT(m_uniformLocationHeightplotScale_transferK != -1);

    m_uniformLocationHeightplotClamp_projection = m_shaderProgramHeightplotClamp.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationHeightplotClamp_projection != -1);
    m_uniformLocationHeightplotClamp_view = m_shaderProgramHeightplotClamp.uniformLocation("viewTransform");
    Q_ASSERT(m_uniformLocationHeightplotClamp_view != -1);
    m_uniformLocationHeightplotClamp_normal = m_shaderProgramHeightplotClamp.uniformLocation("normalTransform");
    Q_ASSERT(m_uniformLocationHeightplotClamp_normal != -1);

    m_uniformLocationHeightplotClamp_material = m_shaderProgramHeightplotClamp.uniformLocation("material");
    Q_ASSERT(m_uniformLocationHeightplotClamp_material != -1);
    m_uniformLocationHeightplotClamp_light = m_shaderProgramHeightplotClamp.uniformLocation("lightPosition");
    Q_ASSERT(m_uniformLocationHeightplotClamp_light != -1);

    m_uniformLocationHeightplotClamp_texture = m_shaderProgramHeightplotClamp.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationHeightplotClamp_texture != -1);

    qDebug() << "m_shaderProgramHeightplotClamp initialized.";
}

void Visualization::opengl_createShaderProgramLic()
{
    m_shaderProgramLic.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/lic.vert");
    m_shaderProgramLic.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/lic.frag");
    m_shaderProgramLic.link();

    m_uniformLocationLic_projection = m_shaderProgramLic.uniformLocation("projectionTransform");
    Q_ASSERT(m_uniformLocationLic_projection != -1);

    m_uniformLocationLicTexture = m_shaderProgramLic.uniformLocation("textureSampler");
    Q_ASSERT(m_uniformLocationLicTexture != -1);

    m_shaderProgramLic.bind();

    qDebug() << "m_shaderProgramLic initialized.";
}

void Visualization::opengl_createShaderProgramVolumeRendering()
{
    m_shaderProgramVolumeRendering.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/volume_rendering.vert");
    m_shaderProgramVolumeRendering.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/volume_rendering2.frag");
    m_shaderProgramVolumeRendering.link();

    m_uniformLocationVolumeRendering_iTime = m_shaderProgramVolumeRendering.uniformLocation("iTime");
    Q_ASSERT(m_uniformLocationVolumeRendering_iTime != -1);

    m_uniformLocationVolumeRendering_iResolution = m_shaderProgramVolumeRendering.uniformLocation("iResolution");
    Q_ASSERT(m_uniformLocationVolumeRendering_iResolution != -1);

    m_shaderProgramVolumeRendering.bind();

    qDebug() << "m_shaderProgramVolumeRendering initialized.";
}

void Visualization::opengl_loadVectorDataTexture(std::vector<Color> const &colorMap)
{
    glBindTexture(GL_TEXTURE_1D, m_vectorDataTextureLocation);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage1D(GL_TEXTURE_1D,
                 0,
                 GL_RGB32F,
                 static_cast<GLint>(colorMap.size()),
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 colorMap.data());
}

void Visualization::opengl_loadLicTexture(std::vector<uint8_t> const &licTexture)
{
    glBindTexture(GL_TEXTURE_2D, m_licTextureLocation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 static_cast<GLint>(m_licObject.getXDim()),
                 static_cast<GLint>(m_licObject.getYDim()),
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 licTexture.data());
}

void Visualization::opengl_updateScalarPoints()
{
    // Recompute and upload grid coordinates.
    std::vector<QVector2D> scalarPoints;
    scalarPoints.reserve(m_DIM * m_DIM);

    for (size_t j = 0U; j < m_DIM; ++j)
    {
        for (size_t i = 0U; i < m_DIM; ++i)
        {
            auto const iFloat = static_cast<float>(i);
            auto const jFloat = static_cast<float>(j);

            QVector2D const v0{m_cellWidth  + iFloat * m_cellWidth,
                               m_cellHeight + jFloat * m_cellHeight};

            scalarPoints.push_back(v0);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vboScalarPoints);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(scalarPoints.size() * 2U * sizeof(float)),
                    scalarPoints.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotPoints);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(scalarPoints.size() * 2U * sizeof(float)),
                    scalarPoints.data());
}

void Visualization::opengl_updateLicPoints()
{
    // Recompute and upload grid coordinates for the quad that the LIC texture is rendered to.
    std::vector<QVector2D> licCoordsAndTexCoords;
    licCoordsAndTexCoords.reserve(8U);

    float min = m_cellWidth;
    float max = static_cast<float>(m_DIM) * m_cellWidth;

    // Top left OpenGL coordinate.
    licCoordsAndTexCoords.emplace_back(QVector2D{min, max});
    licCoordsAndTexCoords.emplace_back(QVector2D{0.0F, 1.0F});

    // Bottom left
    licCoordsAndTexCoords.emplace_back(QVector2D{min, min});
    licCoordsAndTexCoords.emplace_back(QVector2D{0.0F, 0.0F});

    // Top right
    licCoordsAndTexCoords.emplace_back(QVector2D{max, max});
    licCoordsAndTexCoords.emplace_back(QVector2D{1.0F, 1.0F});

    // Bottom right
    licCoordsAndTexCoords.emplace_back(QVector2D{max, min});
    licCoordsAndTexCoords.emplace_back(QVector2D{1.0F, 0.0F});

    glBindBuffer(GL_ARRAY_BUFFER, m_vboLic);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(licCoordsAndTexCoords.size() *sizeof(QVector2D)),
                    licCoordsAndTexCoords.data());
}

void Visualization::opengl_drawScalarData(std::vector<float> const &scalarValues)
{
    switch (m_currentMappingType)
    {
        case MappingType::Scaling:
        {
            if (m_useCustomColorMap)
            {
                m_shaderProgramScalarDataScaleCustomColorMap.bind();
                glUniformMatrix4fv(m_uniformLocationScalarDataScaleCustomColorMap_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());

                auto const currentMinMaxIt = std::minmax_element(scalarValues.cbegin(), scalarValues.cend());
                QVector2D currentMinMax{*currentMinMaxIt.first, *currentMinMaxIt.second};

                m_minMaxDensity.update(currentMinMax);
                QVector2D const minMaxAverage{m_minMaxDensity.average()};

                // Send values to GUI.
                if (m_sendMinMaxToUI)
                {
                    auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
                    Q_ASSERT(mainWindowPtr != nullptr);
                    mainWindowPtr->setScalarDataMin(minMaxAverage.x());
                    mainWindowPtr->setScalarDataMax(minMaxAverage.y());
                }

                glUniform1f(m_uniformLocationScalarDataScaleCustomColorMap_rangeMin, minMaxAverage.x());
                glUniform1f(m_uniformLocationScalarDataScaleCustomColorMap_rangeMax, minMaxAverage.y());
                glUniform1f(m_uniformLocationScalarDataScaleCustomColorMap_transferK, m_transferK);

                GLfloat const *ptrToFirstElement = &m_customColors[0].r;
                glUniform3fv(m_uniformLocationScalarDataScaleCustomColorMap_colorMapColors, 3, ptrToFirstElement);
            }
            else
            {
                m_shaderProgramScalarDataScaleTexture.bind();
                glUniformMatrix4fv(m_uniformLocationScalarDataScaleTexture_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());

                auto const currentMinMaxIt = std::minmax_element(scalarValues.cbegin(), scalarValues.cend());
                QVector2D currentMinMax{*currentMinMaxIt.first, *currentMinMaxIt.second};

                m_minMaxDensity.update(currentMinMax);
                QVector2D const minMaxAverage{m_minMaxDensity.average()};

                // Send values to GUI.
                if (m_sendMinMaxToUI)
                {
                    auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
                    Q_ASSERT(mainWindowPtr != nullptr);
                    mainWindowPtr->setScalarDataMin(minMaxAverage.x());
                    mainWindowPtr->setScalarDataMax(minMaxAverage.y());
                }

                glUniform1f(m_uniformLocationScalarDataScaleTexture_rangeMin, minMaxAverage.x());
                glUniform1f(m_uniformLocationScalarDataScaleTexture_rangeMax, minMaxAverage.y());
                glUniform1f(m_uniformLocationScalarDataScaleTexture_transferK, m_transferK);

                glUniform1i(m_uniformLocationScalarDataScaleTexture_texture, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_1D, m_scalarDataTextureLocation);
            }
        }
        break;

        case MappingType::Clamping:
        {
            if (m_useCustomColorMap)
            {
                m_shaderProgramScalarDataClampCustomColorMap.bind();
                glUniformMatrix4fv(m_uniformLocationScalarDataClampCustomColorMap_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());

                // Send values to GUI.
                if (m_sendMinMaxToUI)
                {
                    auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
                    Q_ASSERT(mainWindowPtr != nullptr);
                    mainWindowPtr->setScalarDataMin(m_clampMin);
                    mainWindowPtr->setScalarDataMax(m_clampMax);
                }

                glUniform1f(m_uniformLocationScalarDataClampCustomColorMap_clampMin, m_clampMin);
                glUniform1f(m_uniformLocationScalarDataClampCustomColorMap_clampMax, m_clampMax);
                glUniform1f(m_uniformLocationScalarDataClampCustomColorMap_transferK, m_transferK);

                GLfloat const *ptrToFirstElement = &m_customColors[0].r;
                glUniform3fv(m_uniformLocationScalarDataClampCustomColorMap_colorMapColors, 3, ptrToFirstElement);
            }
            else
            {
                m_shaderProgramScalarDataClampTexture.bind();
                glUniformMatrix4fv(m_uniformLocationScalarDataClampTexture_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());

                // Send values to GUI.
                if (m_sendMinMaxToUI)
                {
                    auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
                    Q_ASSERT(mainWindowPtr != nullptr);
                    mainWindowPtr->setScalarDataMin(m_clampMin);
                    mainWindowPtr->setScalarDataMax(m_clampMax);
                }

                glUniform1f(m_uniformLocationScalarDataClampTexture_clampMin, m_clampMin);
                glUniform1f(m_uniformLocationScalarDataClampTexture_clampMax, m_clampMax);
                glUniform1f(m_uniformLocationScalarDataClampTexture_transferK, m_transferK);

                glUniform1i(m_uniformLocationScalarDataClampTexture_texture, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_1D, m_scalarDataTextureLocation);
            }
        }
        break;
    }

    glBindVertexArray(m_vaoScalarData);

    // Copy scalars to GPU buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboScalarData);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(scalarValues.size() * sizeof(float)),
                    scalarValues.data());

    glDrawElements(GL_TRIANGLE_STRIP,
                   static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_SHORT,
                   static_cast<GLvoid*>(nullptr));
}

void Visualization::opengl_drawIsolines()
{

    float const stepsize = [&]()
    {
        if (m_numberOfIsolines > 1)
            return (m_isolineMaxValue - m_isolineMinValue) / (m_numberOfIsolines - 1);

        return 1.0F;
    }();

    std::vector<float> scalarValues;

    switch (m_manuallyChooseIsolineDataType ? m_currentIsolineDataType : m_currentScalarDataType)
    {
        case ScalarDataType::Density:
            scalarValues = m_simulation.density();
        break;

        case ScalarDataType::ForceFieldMagnitude:
            scalarValues = m_simulation.forceFieldMagnitude();
        break;

        case ScalarDataType::VelocityMagnitude:
            scalarValues = m_simulation.velocityMagnitude();
        break;

        case ScalarDataType::VelocityDivergence:
            scalarValues = velocityDivergence();
        break;

        case ScalarDataType::ForceFieldDivergence:
            scalarValues = forceFieldDivergence();
        break;
    }


    for (size_t n = 0U; n < m_numberOfIsolines; ++n)
    {
        float const currentIsolineValue = m_isolineMinValue + (n * stepsize);
        std::vector<QVector2D> const vertices{Isoline(scalarValues,
                                                      m_DIM,
                                                      currentIsolineValue,
                                                      m_cellWidth,
                                                      m_isolineInterpolationMethod,
                                                      m_isolineAmbiguousCaseDecider).vertices()};

        std::vector<QVector3D> isolineVertices;
        isolineVertices.reserve(vertices.size());
        for (auto const &v : vertices)
            isolineVertices.emplace_back(v, 0.0F);

        glBindVertexArray(m_vaoIsolines);


        // buffer data and draw lines
        glBindBuffer(GL_ARRAY_BUFFER, m_vboIsolines);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(isolineVertices.size() * sizeof(QVector3D)),
                     isolineVertices.data(),
                     GL_DYNAMIC_DRAW);


        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(isolineVertices.size()));
    }
}

void Visualization::opengl_drawHeightplot()
{
    std::vector<float> scalarValues;
    std::vector<float> heightValues;

    switch (m_currentScalarDataType)
    {
        case ScalarDataType::Density:
            scalarValues = m_simulation.density();
        break;

        case ScalarDataType::ForceFieldMagnitude:
            scalarValues = m_simulation.forceFieldMagnitude();
        break;

        case ScalarDataType::VelocityMagnitude:
            scalarValues = m_simulation.velocityMagnitude();
        break;

        case ScalarDataType::VelocityDivergence:
            scalarValues = velocityDivergence();
        break;

        case ScalarDataType::ForceFieldDivergence:
            scalarValues = forceFieldDivergence();
        break;
    }

    switch (m_currentHeightplotDataType)
    {
        case ScalarDataType::Density:
            heightValues = m_simulation.density();
        break;

        case ScalarDataType::ForceFieldMagnitude:
            heightValues = m_simulation.forceFieldMagnitude();
        break;

        case ScalarDataType::VelocityMagnitude:
            heightValues = m_simulation.velocityMagnitude();
        break;

        case ScalarDataType::VelocityDivergence:
            heightValues = velocityDivergence();
        break;

        case ScalarDataType::ForceFieldDivergence:
            heightValues = forceFieldDivergence();
        break;
    }

    switch (m_currentMappingType)
    {
        case MappingType::Scaling:
        {
            m_shaderProgramHeightplotScale.bind();
            glUniformMatrix4fv(m_uniformLocationHeightplotScale_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());
            glUniformMatrix4fv(m_uniformLocationHeightplotScale_view, 1, GL_FALSE, m_viewTransformationMatrix.data());
            glUniformMatrix3fv(m_uniformLocationHeightplotScale_normal, 1, GL_FALSE, m_normalTransformationMatrix.data());

            glUniform4fv(m_uniformLocationHeightplotScale_material, 1, &m_materialConstants[0]);
            glUniform3fv(m_uniformLocationHeightplotScale_light, 1, &m_lightPosition[0]);

            auto const currentMinMaxIt = std::minmax_element(scalarValues.cbegin(), scalarValues.cend());
            QVector2D currentMinMax{*currentMinMaxIt.first, *currentMinMaxIt.second};

            m_minMaxDensity.update(currentMinMax);
            QVector2D const minMaxAverage{m_minMaxDensity.average()};

            // Send values to GUI.
            if (m_sendMinMaxToUI)
            {
                auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
                Q_ASSERT(mainWindowPtr != nullptr);
                mainWindowPtr->setScalarDataMin(minMaxAverage.x());
                mainWindowPtr->setScalarDataMax(minMaxAverage.y());
            }

            glUniform1f(m_uniformLocationHeightplotScale_rangeMin, minMaxAverage.x());
            glUniform1f(m_uniformLocationHeightplotScale_rangeMax, minMaxAverage.y());
            glUniform1f(m_uniformLocationHeightplotScale_transferK, m_transferK);

            glUniform1i(m_uniformLocationHeightplotScale_texture, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_1D, m_scalarDataTextureLocation);
        }
        break;

        case MappingType::Clamping:
        {
            m_shaderProgramHeightplotClamp.bind();
            glUniformMatrix4fv(m_uniformLocationHeightplotClamp_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());
            glUniformMatrix4fv(m_uniformLocationHeightplotClamp_view, 1, GL_FALSE, m_viewTransformationMatrix.data());
            glUniformMatrix3fv(m_uniformLocationHeightplotClamp_normal, 1, GL_FALSE, m_normalTransformationMatrix.data());

            glUniform4fv(m_uniformLocationHeightplotClamp_material, 1, &m_materialConstants[0]);
            glUniform3fv(m_uniformLocationHeightplotClamp_light, 1, &m_lightPosition[0]);

            // Send values to GUI.
            auto const mainWindowPtr = qobject_cast<MainWindow*>(parent()->parent());
            Q_ASSERT(mainWindowPtr != nullptr);
            mainWindowPtr->setScalarDataMin(m_clampMin);
            mainWindowPtr->setScalarDataMax(m_clampMax);

            glUniform1f(m_uniformLocationHeightplotClamp_clampMin, m_clampMin);
            glUniform1f(m_uniformLocationHeightplotClamp_clampMax, m_clampMax);
            glUniform1f(m_uniformLocationHeightplotClamp_transferK, m_transferK);

            glUniform1i(m_uniformLocationHeightplotClamp_texture, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_1D, m_scalarDataTextureLocation);
        }
        break;
    }

    // Add scaling for nicer results
    std::transform(heightValues.begin(), heightValues.end(), heightValues.begin(),
                   [](float x){ return x * 3.0F; });

    std::vector<QVector3D> normals = computeNormals(heightValues);

    glBindVertexArray(m_vaoHeightplot);

    // Copy scalars to GPU buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotScalarValues);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(scalarValues.size() * sizeof(float)),
                    scalarValues.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotHeight);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(heightValues.size() * sizeof(float)),
                    heightValues.data());

    glBindBuffer(GL_ARRAY_BUFFER, m_vboHeightplotNormals);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(normals.size() * 3 * sizeof(float)),
                    normals.data());

    glDrawElements(GL_TRIANGLE_STRIP,
                   static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_SHORT,
                   static_cast<GLvoid*>(nullptr));
}

void Visualization::opengl_drawLic()
{
    std::vector<float> vectorField_in_x;
    std::vector<float> vectorField_in_y;
    vectorField_in_x = m_simulation.velocityXInterpolated(m_licObject.getXDim(), m_licObject.getYDim()); // These should get the force field vectors of size equal to the simulation area, if I understand the function correctly?
    vectorField_in_y = m_simulation.velocityYInterpolated(m_licObject.getXDim(), m_licObject.getYDim());

    //m_licObject.resetTexture(); // Uncomment this line if you want the noise texture to look like its "Flowing".

    std::vector<float> texture_in = m_licObject.getTexture();

    std::vector<uint8_t> texture_out = m_licObject.updateTexture(vectorField_in_x, vectorField_in_y, texture_in); //Generate the texture to be sent to openGL

    /*
    if (m_lic_testing_iterator % 10 == 0) // print out image every 10 steps (just so that it doesn't spam out images every millisecond), don't keep this in for openGL
        m_licObject.writeImage(texture_out); // For testing, delete later

    ++m_lic_testing_iterator; // can get rid of this too
    // do openGL stuff here.
    */
    m_shaderProgramLic.bind();
    glUniformMatrix4fv(m_uniformLocationLic_projection, 1, GL_FALSE, m_projectionTransformationMatrix.data());
    glUniform1i(m_uniformLocationLicTexture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_licTextureLocation);

    opengl_loadLicTexture(texture_out);

    glBindVertexArray(m_vaoLic);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Visualization::opengl_drawVolumeRendering()
{
    m_shaderProgramVolumeRendering.bind();
    std::array<float, 3U> const iResolution{static_cast<float>(width()),
                                            static_cast<float>(height()),
                                            static_cast<float>(screen()->devicePixelRatio())};

    glUniform3fv(m_uniformLocationVolumeRendering_iResolution, 1, iResolution.data());
    glUniform1f(m_uniformLocationVolumeRendering_iTime, m_elapsedTimer.elapsed() / 1000.0F);

    glBindVertexArray(m_vaoVolumeRendering);
    glDrawElements(GL_TRIANGLES,
                   m_indices.size(),
                   GL_UNSIGNED_SHORT,
                   static_cast<GLvoid*>(nullptr));
}

void Visualization::opengl_rotateView()
{
    m_viewTransformationMatrix.setToIdentity();
    m_viewTransformationMatrix.rotate(m_rotation.x(), 1.0F, 0.0F, 0.0F);
    m_viewTransformationMatrix.rotate(m_rotation.y(), 0.0F, 1.0F, 0.0F);
    m_viewTransformationMatrix.rotate(m_rotation.z(), 0.0F, 0.0F, 1.0F);
}
