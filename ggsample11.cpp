// ウィンドウ関連の処理
#include "GgApplication.h"

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
  { 0.0f, 0.0f, 1.0f, 1.0f }  // 光源位置
};

// 物体の材質
const GgSimpleShader::Material objectMaterial =
{
  { 0.8f, 0.8f, 0.8f, 1.0f }, // 環境光に対する反射係数
  { 0.8f, 0.8f, 0.8f, 0.0f }, // 拡散反射係数
  { 0.2f, 0.2f, 0.2f, 0.0f }, // 鏡面反射係数
  40.0f                       // 輝き係数
};

// 影の材質
const GgSimpleShader::Material shadowMaterial =
{
  { 0.2f, 0.2f, 0.2f, 1.0f }, // 環境光に対する反射係数
  { 0.0f, 0.0f, 0.0f, 0.0f }, // 拡散反射係数
  { 0.0f, 0.0f, 0.0f, 0.0f }, // 鏡面反射係数
  0.0f                        // 輝き係数
};

// ワールド座標系の光源位置
const GgVector lp = { 0.0f, 4.0f, 0.0f, 1.0f };

// アニメーションの変換行列を求める
static GgMatrix animate(GLfloat t, int i)
{
  const GLfloat h(fmod(36.0f * t, 2.0f) - 1.0f);
  const GLfloat x(0.0f), y(1.0f - h * h), z(1.5f);
  const GLfloat r(static_cast<GLfloat>(M_PI * (2.0 * i / objects - 4.0 * t)));

  return ggRotateY(r).translate(x, y, z);
}

//
// アプリケーションの実行
//
void GgApplication::run()
{
  // ウィンドウを作成する
  Window window("ggsample11");

  // プログラムオブジェクトの作成
  GgSimpleShader shader("ggsample11.vert", "ggsample11.frag");

  // 床の図形データの読み込み
  const GgSimpleObj floor("floor.obj");

  // 丸影用の楕円の作成
  const std::unique_ptr<const GgTriangles> ellipse(ggEllipse(0.8f, 0.6f, 24));

  // 影の材質バッファ
  const GgSimpleShader::MaterialBuffer materialBuffer(shadowMaterial);

  // 物体の図形データの読み込み
  const std::unique_ptr<const GgElements> object(ggElementsObj("bunny.obj"));

  // 物体の材質
  GgSimpleShader::MaterialBuffer objectMaterialBuffer(objectMaterial, objects);
  for (int i = 0; i < objects; ++i)
  {
    // 個々の物体の色を決める
    const int j(i % 6 + 1);
    const GLfloat r((j & 1) * 0.4f + 0.4f);
    const GLfloat g((j & 2) * 0.2f + 0.4f);
    const GLfloat b((j & 4) * 0.1f + 0.4f);

    // 物体の色を設定する
    objectMaterialBuffer.loadAmbientAndDiffuse(r, g, b, 1.0f, i);
  }

  // ビュー変換行列を mv に求める
  const GgMatrix mv(ggLookat(0.0f, 3.0f, 8.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

  //
  // 影つけ処理の設定
  //

  // 視点座標系の光源位置を求める
  light.position = mv * lp;

  // 光源
  const GgSimpleShader::LightBuffer lightBuffer(light);

  // 影付け処理用の変換行列 (丸影に使う楕円を床に張り付けるために y 座標値に 0 をかける)
  //   【宿題】これを Projection Shadow 用の変換行列に置き換える
  const GLfloat m[] =
  {
      1.0f,   0.0f,   0.0f,   0.0f,
      0.0f,   0.0f,   0.0f,   0.0f,
      0.0f,   0.0f,   1.0f,   0.0f,
      0.0f,   0.0f,   0.0f,   1.0f
  };
  const GgMatrix ms(m);

  //
  // その他の設定
  //

  // 背景色の設定
  glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

  // 隠面消去の設定
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // 経過時間のリセット
  glfwSetTime(0.0);

  // ウィンドウが開いている間くり返し描画する
  while (window)
  {
    // 時刻の計測
    const float t(static_cast<float>(fmod(glfwGetTime(), cycle) / cycle));

    // 投影変換行列
    const GgMatrix mp(ggPerspective(0.5f, window.getAspect(), 1.0f, 15.0f));

    // シェーダプログラムの使用開始
    shader.use(lightBuffer);

    // 画面消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // 1. 最初に床を描画します
    //

    // 床の描画
    shader.loadMatrix(mp, mv);
    floor.draw();

    //
    // 2. 影のオブジェクトを床の高さに描画します
    //      影のオブジェクトが確実に描画されるよう隠面消去処理を無効します
    //

    // 影の材質
    materialBuffer.select();

    // 影の描画
    glDisable(GL_DEPTH_TEST);
    for (int i = 0; i < objects; ++i)
    {
      // アニメーションの変換行列
      const GgMatrix ma(animate(t, i));

      // 影の描画 (楕円は XY 平面上にあるので X 軸中心に -π/2 回転)
      //   【宿題】楕円の代わりに影を落とす図形そのものを描く (-π/2 回転は不要)
      shader.loadModelviewMatrix(mv * ms * ma * ggRotateX(-1.570796f));
      ellipse->draw();
    }
    glEnable(GL_DEPTH_TEST);

    //
    // 3. 図形を描画します
    //

    // シェーダプログラムの使用開始 (時刻 t にもとづく回転アニメーション)
    for (int i = 0; i < objects; ++i)
    {
      // アニメーションの変換行列
      const GgMatrix ma(animate(t, i));

      // 図形の材質
      objectMaterialBuffer.select(i);

      // 図形の描画
      shader.loadModelviewMatrix(mv * ma);
      object->draw();
    }

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
