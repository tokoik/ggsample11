// ウィンドウ関連の処理
#include "Window.h"

// 標準ライブラリ
#include <cmath>
#include <memory>

// アニメーションの周期（秒）
const double cycle(10.0);

// オブジェクトの数
const int objects(6);

// 光源
GgSimpleShader::Light light =
{
  { 0.2f, 0.2f, 0.2f, 1.0f }, // 環境光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 拡散反射光成分
  { 1.0f, 1.0f, 1.0f, 0.0f }, // 鏡面反射光成分
  { 0.0f, 0.0f, 1.0f, 1.0f }  // 視点座標系の光源位置
};

// ワールド座標系の光源位置
const GLfloat lp[] = { 0.0f, 4.0f, 0.0f, 1.0f };

// アニメーションの変換行列を求める
static GgMatrix animate(GLfloat t, int i)
{
  const GLfloat h(fmod(36.0f * t, 2.0f) - 1.0f);
  const GLfloat x(0.0f), y(1.0f - h * h), z(1.5f);
  const GLfloat r(static_cast<GLfloat>(M_PI * (2.0 * i / objects - 4.0 * t)));

  return ggRotateY(r).translate(x, y, z);
}

//
// メインプログラム
//
int main(int argc, const char * argv[])
{
  // ウィンドウを作成する
  Window window("ggsample11");

  // 背景色の設定
  glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

  // 隠面消去の設定
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // プログラムオブジェクトの作成
  GgSimpleShader shader("simple.vert", "simple.frag");

  // ビュー変換行列を mv に求める
  const GgMatrix mv(ggLookat(0.0f, 3.0f, 8.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

  // 視点座標系の光源位置を求める
  mv.projection(light.position, lp);

  // 影付け処理用の変換行列 (影の高さを 0 にするために y 座標値に 0 をかける)
  const GgMatrix ms(ggScale(1.0f, 0.0f, 1.0f));

  // 図形の読み込み
  const GgObj floor("floor.obj");
  const std::unique_ptr<const GgElements> object(ggElementsObj("bunny.obj"));

  // 丸影用の楕円
  const std::unique_ptr<const GgTriangles> ellipse(ggEllipse(0.8f, 0.6f, 24));

  // 経過時間のリセット
  glfwSetTime(0.0);

  // ウィンドウが開いている間くり返し描画する
  while (window.shouldClose() == GL_FALSE)
  {
    // 時刻の計測
    const float t(static_cast<float>(fmod(glfwGetTime(), cycle) / cycle));

    // 投影変換行列
    const GgMatrix mp(ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f));

    // シェーダプログラムの使用開始
    shader.use();
    shader.loadLight(light);

    // 画面消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 床の描画
    shader.loadMatrix(mp, mv);
    floor.draw(shader);

    // 影の材質
    shader.loadMaterialAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    shader.loadMaterialDiffuse(0.0f, 0.0f, 0.0f, 0.0f);
    shader.loadMaterialSpecular(0.0f, 0.0f, 0.0f, 0.0f);
    shader.loadMaterialShininess(0.0f);

    // 影の描画
    glDisable(GL_DEPTH_TEST);
    for (int i = 1; i <= objects; ++i)
    {
      // アニメーションの変換行列
      const GgMatrix ma(animate(t, i));

      // 影の描画 (楕円は XY 平面上にあるので X 軸中心に -π/2 回転)
      shader.loadMatrix(mp, mv * ms * ma * ggRotateX(-1.570796f));
      ellipse->draw();
    }
    glEnable(GL_DEPTH_TEST);

    // シェーダプログラムの使用開始 (時刻 t にもとづく回転アニメーション)
    for (int i = 1; i <= objects; ++i)
    {
      // アニメーションの変換行列
      const GgMatrix ma(animate(t, i));

      // オブジェクトの色
      const GLfloat color[] =
      {
        (i & 1) * 0.4f + 0.4f,
        (i & 2) * 0.2f + 0.4f,
        (i & 4) * 0.1f + 0.4f,
        1.0f
      };

      // 図形の材質
      shader.loadMaterialAmbient(color);
      shader.loadMaterialDiffuse(color);
      shader.loadMaterialSpecular(0.2f, 0.2f, 0.2f, 0.0f);
      shader.loadMaterialShininess(40.0f);

      // 図形の描画
      shader.loadMatrix(mp, mv * ma);
      object->draw();
    }

    // シェーダプログラムの使用終了
    glUseProgram(0);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return 0;
}
