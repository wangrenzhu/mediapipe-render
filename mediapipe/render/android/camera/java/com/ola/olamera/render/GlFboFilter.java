package com.ola.olamera.render;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.PointF;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;


import com.ola.olamera.util.GlCommonUtil;
import com.ola.olamera.util.MatrixUtils;
import com.ola.olamera.util.OpenGlUtils;

import java.nio.FloatBuffer;
import java.util.LinkedList;

import static com.ola.olamera.render.CropFboFilter.doSnapshot;


@TargetApi(18)
//TODO 后面将filter相同内容抽象成基类

public class GlFboFilter {

    private final FloatBuffer mVtxBuf = OpenGlUtils.createSquareVtx();
    protected float[] mPosMtx = OpenGlUtils.createIdentityMtx();
    private float[] mNormalPosMtx = OpenGlUtils.createIdentityMtx();
    private float[] mIdentityMtx = OpenGlUtils.createIdentityMtx();
    private float[] mFlipXPosMtx = MatrixUtils.flipF(OpenGlUtils.createIdentityMtx(), true, false);
    private final float[] mFlipYPosMtx = MatrixUtils.flipF(OpenGlUtils.createIdentityMtx(), false, true);

    protected int mInputTextureId = -1;
    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muPosMtxHandle = -1;
    private int muTexMtxHandle = -1;
    private int mSingleStepOffsetHandler = -1;

    private final int[] mTexId = new int[]{0};

    private int mFboId = -1;
    private int mRboId = -1;

    protected int mInputWidth = -1;
    protected int mInputHeight = -1;

    protected int mOutputWidth = -1;
    protected int mOutputHeight = -1;

    private boolean mIsExternalOES;//是否从外部纹理读取数据

    private final LinkedList<Runnable> mRunOnDraw;
    private String mVertex;
    private String mFragment;

    private int mX = 0;
    private int mY = 0;
    private boolean mNeedClear = true;

    //TODO only for test code，always false when commit
    boolean mDoSnapshot = false;

    private boolean mPrepared = false;


    public GlFboFilter() {
        this(true);
    }

    public GlFboFilter(boolean isExternalOES) {
        mIsExternalOES = isExternalOES;
        mRunOnDraw = new LinkedList<>();
        if (isExternalOES) {
            mVertex = GLConstant.SHADER_DEFAULT_VERTEX;
            mFragment = GLConstant.SHADER_DEFAULT_FRAGMENT_OES;
        } else {
            mVertex = GLConstant.SHADER_DEFAULT_VERTEX;
            mFragment = GLConstant.SHADER_DEFAULT_FRAGMENT_NOT_OES;
        }
    }

    public void normalPosMtx() {
        mPosMtx = mNormalPosMtx;
    }

    public void flipPosMtxX() {
        mPosMtx = mFlipXPosMtx;
    }

    public void flipPosMtxY() {
        mPosMtx = mFlipYPosMtx;
    }

    public void setInputSize(int width, int height) {
        mInputWidth = width;
        mInputHeight = height;
        mOutputWidth = mInputWidth;
        mOutputHeight = mInputHeight;
    }

    public void setXY(int x, int y) {
        mX = x;
        mY = y;
    }

    public void setOutputSize(int width, int height) {
        mOutputWidth = width;
        mOutputHeight = height;
    }

    public int getOutputWidth() {
        return mOutputWidth;
    }

    public int getOutputHeight() {
        return mOutputHeight;
    }

    public void setNeedClear(boolean need) {
        mNeedClear = need;
    }

    public void clear() {
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
    }

    public void prepare() {
        Log.d("e", "prepare createFrameBuffer" + mPrepared + ", width " + mInputWidth + ", height" + mInputHeight);
        if (mPrepared) {
            return;
        }

        loadShaderAndParams(mVertex, mFragment);
        createEffectTexture();
        mPrepared = true;
    }

    public void setInputTextureId(int textureId) {
        mInputTextureId = textureId;
    }

    private void loadShaderAndParams(String vertex, String fragment) {
        GlCommonUtil.checkGlError("initSH_S");
        mProgram = GlCommonUtil.createProgram(vertex, fragment);
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "position");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "inputTextureCoordinate");
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muTexMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexMtx");
        mSingleStepOffsetHandler = GLES20.glGetUniformLocation(mProgram, "singleStepOffset");

        if (mSingleStepOffsetHandler != -1) {
            setFloatVec2(mSingleStepOffsetHandler, new float[]{2.5f / mInputWidth, 2.5f / mInputHeight});
        }
        GlCommonUtil.checkGlError("initSH_E");
    }

    public int getInputWidth() {
        return mInputWidth;
    }

    public int getInputHeight() {
        return mInputHeight;
    }

    private void createEffectTexture() {
        if (mInputWidth <= 0 || mInputHeight <= 0) {
            return;
        }
        GlCommonUtil.checkGlError("initFBO_S");
        createFrameBuffer();
        GLES20.glGenTextures(1, mTexId, 0);

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId);


        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTexId[0]);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA,
                mOutputWidth, mOutputHeight, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);

        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER,
                GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, mTexId[0], 0);

        if (GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER) !=
                GLES20.GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("glCheckFramebufferStatus()");
        }
        GlCommonUtil.checkGlError("initFBO_E");
    }


    public int getOutputTextureId() {
        return mTexId[0];
    }

    protected void runOnDraw(final Runnable runnable) {
        synchronized (mRunOnDraw) {
            mRunOnDraw.addLast(runnable);
        }
    }

    protected void runPendingOnDrawTasks() {
        while (!mRunOnDraw.isEmpty()) {
            mRunOnDraw.removeFirst().run();
        }
    }

    public void draw(float[] tex_mtx) {
        if (tex_mtx == null) {
            tex_mtx = mIdentityMtx;
        }
        draw(tex_mtx, mPosMtx);
    }

    public void draw(float[] tex_mtx, final float[] posMatrix) {
        if (-1 == mProgram || mInputTextureId == -1 || mInputWidth == -1) {
            return;
        }

        GlCommonUtil.checkGlError("draw_S");
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId);

        GlCommonUtil.checkGlError("draw_S");

        GLES20.glViewport(mX, mY, mOutputWidth, mOutputHeight);

        GlCommonUtil.checkGlError("draw_S");

        if (mNeedClear) {
            GLES20.glClearColor(0f, 0f, 0f, 0f);
            GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        }
        GLES20.glUseProgram(mProgram);

        GlCommonUtil.checkGlError("draw_S");

        runPendingOnDrawTasks();

        GlCommonUtil.checkGlError("draw_S");

        mVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle,
                3, GLES20.GL_FLOAT, false, 4 * (3 + 2), mVtxBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);

        GlCommonUtil.checkGlError("draw_S");

        mVtxBuf.position(3);
        GLES20.glVertexAttribPointer(maTexCoordHandle,
                2, GLES20.GL_FLOAT, false, 4 * (3 + 2), mVtxBuf);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);

        GlCommonUtil.checkGlError("draw_S");

        if (muPosMtxHandle >= 0) {
            GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, posMatrix, 0);
        }

        GlCommonUtil.checkGlError("draw_S");

        if (muTexMtxHandle >= 0) {
            GLES20.glUniformMatrix4fv(muTexMtxHandle, 1, false, tex_mtx, 0);
        }

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        if (mIsExternalOES) {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mInputTextureId);
        } else {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mInputTextureId);
        }

        GlCommonUtil.checkGlError("draw_S");

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        //only for test code
        if (mDoSnapshot) {
            Bitmap result = doSnapshot(mOutputWidth, mOutputHeight);
            Log.e("SnapShotTest", "test  " + result.getHeight());
        }


        GlCommonUtil.checkGlError("draw_S");

        GLES20.glDisableVertexAttribArray(maPositionHandle);
        GLES20.glDisableVertexAttribArray(maTexCoordHandle);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        if (mIsExternalOES) {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        } else {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }
        GLES20.glUseProgram(0);

        GlCommonUtil.checkGlError("draw_E");
    }

    protected void setInteger(final int location, final int intValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1i(location, intValue);
            }
        });
    }

    protected void setFloat(final int location, final float floatValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1f(location, floatValue);
            }
        });
    }

    protected void setFloatVec2(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform2fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatVec3(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform3fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatVec4(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform4fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatArray(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1fv(location, arrayValue.length, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setPoint(final int location, final PointF point) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                float[] vec2 = new float[2];
                vec2[0] = point.x;
                vec2[1] = point.y;
                GLES20.glUniform2fv(location, 1, vec2, 0);
            }
        });
    }

    protected void setUniformMatrix3f(final int location, final float[] matrix) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                GLES20.glUniformMatrix3fv(location, 1, false, matrix, 0);
            }
        });
    }

    protected void setUniformMatrix4f(final int location, final float[] matrix) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                GLES20.glUniformMatrix4fv(location, 1, false, matrix, 0);
            }
        });
    }

    private void createFrameBuffer() {
        int[] fboId = new int[]{0};
//        int[] rboId = new int[]{0};
        GLES20.glGenFramebuffers(1, fboId, 0);
//        GLES20.glGenRenderbuffers(1, rboId, 0);
        mFboId = fboId[0];
//        mRboId = rboId[0];
        Log.d("e", "createFrameBuffer: ");
    }

    private void releaseFrameBuffer() {
        if (mFboId != -1) {
            int[] fboId = new int[]{mFboId};
            GLES20.glDeleteFramebuffers(1, fboId, 0);
            mFboId = -1;
        }
//        if (mRboId != -1) {
//            int[] rboId = new int[]{mRboId};
//            GLES20.glDeleteRenderbuffers(1, rboId, 0);
//            mRboId = -1;
//        }
    }

    private void releaseProgram() {
        if (mProgram == -1) {
            return;
        }
        GLES20.glDeleteProgram(mProgram);
    }

    public void release() {
        releaseFrameBuffer();
        releaseProgram();
    }
}