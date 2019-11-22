#include <gtest/gtest.h>
#include "utils/HashFunctions.h"
#include "BaseThreadDecoderTest.h"
#include <string>

static void UpdateHashFromPlane (SHA1Context* ctx, const uint8_t* plane,
                                 int width, int height, int stride) {
  for (int i = 0; i < height; i++) {
    SHA1Input (ctx, plane, width);
    plane += stride;
  }
}

class ThreadDecoderCapabilityTest : public ::testing::Test {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F (ThreadDecoderCapabilityTest, JustInit) {
  SDecoderCapability sDecCap;
  int iRet = WelsGetDecoderCapability (&sDecCap);
  ASSERT_TRUE (iRet == 0);
  EXPECT_EQ (sDecCap.iProfileIdc, 66);
  EXPECT_EQ (sDecCap.iProfileIop, 0xE0);
  EXPECT_EQ (sDecCap.iLevelIdc, 32);
  EXPECT_EQ (sDecCap.iMaxMbps, 216000);
  EXPECT_EQ (sDecCap.iMaxFs, 5120);
  EXPECT_EQ (sDecCap.iMaxCpb, 20000);
  EXPECT_EQ (sDecCap.iMaxDpb, 20480);
  EXPECT_EQ (sDecCap.iMaxBr, 20000);
  EXPECT_EQ (sDecCap.bRedPicCap, false);
}


class ThreadDecoderInitTest : public ::testing::Test, public BaseThreadDecoderTest {
 public:
  virtual void SetUp() {
    BaseThreadDecoderTest::SetUp();
  }
  virtual void TearDown() {
    BaseThreadDecoderTest::TearDown();
  }
};

TEST_F (ThreadDecoderInitTest, JustInit) {}

struct FileParam {
  const char* fileName;
  const char* hashStr;
};

class ThreadDecoderOutputTest : public ::testing::WithParamInterface<FileParam>,
  public ThreadDecoderInitTest, public BaseThreadDecoderTest::Callback {
 public:
  virtual void SetUp() {
    ThreadDecoderInitTest::SetUp();
    if (HasFatalFailure()) {
      return;
    }
    SHA1Reset (&ctx_);
  }
  virtual void onDecodeFrame (const Frame& frame) {
    const Plane& y = frame.y;
    const Plane& u = frame.u;
    const Plane& v = frame.v;
    UpdateHashFromPlane (&ctx_, y.data, y.width, y.height, y.stride);
    UpdateHashFromPlane (&ctx_, u.data, u.width, u.height, u.stride);
    UpdateHashFromPlane (&ctx_, v.data, v.width, v.height, v.stride);
  }
 protected:
  SHA1Context ctx_;
};

TEST_P (ThreadDecoderOutputTest, CompareOutput) {
  FileParam p = GetParam();
#if defined(ANDROID_NDK)
  std::string filename = std::string ("/sdcard/") + p.fileName;
  ASSERT_TRUE (ThreadDecodeFile (filename.c_str(), this));
#else
  ASSERT_TRUE (ThreadDecodeFile (p.fileName, this));
#endif

  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1Result (&ctx_, digest);
  if (!HasFatalFailure()) {
    CompareHash (digest, p.hashStr);
  }
}
static const FileParam kFileParamArray[] = {
  {"res/Adobe_PDF_sample_a_1024x768_50Frms.264", "3d94e7d515c857d968f8771e9691b53123e92c59"},
  {"res/BA1_FT_C.264", "cdc231904887d47f5e813cc87568c679e6c617f5"},
  {"res/BA1_Sony_D.jsv", "37c9a951a0348d6abe1880b59e2b5a4d7d18c94c"},
  {"res/BAMQ1_JVC_C.264", "477b1e45e30661a138ff0b43c1ed3e00ded13d9c"},
  {"res/BAMQ2_JVC_C.264", "2c89a880322a7bde152104b0aa7815f3ecd66de2"},
  {"res/BA_MW_D.264", "157dfc7d98dce5e09d513e5c47784510dd6e8c12"},
  {"res/BANM_MW_D.264", "da4a9cf7f0f744a8e02e56afdcf45e7049c47ee7"},
  {"res/BASQP1_Sony_C.jsv", "68e604b77e3f57f8ef1c2e450fcef03f5d2aee90"},
  {"res/CI1_FT_B.264", "70d8f194819e23c824a4141d23232984b1145923"},
  {"res/CI_MW_D.264", "ddc99a77703216fb4d8db52b9ff2f55578f5e688"},
  {"res/CVFC1_Sony_C.jsv", "b86af0f66029e95b39da8f7818dbbe987fe0466c"},
  {"res/CVPCMNL1_SVA_C.264", "c2b0d964de727c64b9fccb58f63b567c82bda95a"},
  //{"res/LS_SVA_D.264", "72118f4d1674cf14e58bed7e67cb3aeed3df62b9"},
  {"res/MIDR_MW_D.264", "31779fbd83b89fe8c59fb8d1fce15393a25e8a61"},
  {"res/MPS_MW_A.264", "0da269a559c8690a9af9635eb2ff3cabcabdda2e"},
  {"res/MR1_BT_A.h264", "b3084391017af3366b80a13a1c3e1811ab2b4f20"},
  {"res/MR1_MW_A.264", "ce59fe038d44e1299a0053de2500f99507e3c3ce"},
  {"res/MR2_MW_A.264", "1073f05bf6af3550d95316f2e26491db90dbe7a5"},
  //{"res/MR2_TANDBERG_E.264", "74d618bc7d9d41998edf4c85d51aa06111db6609"},
  {"res/NL1_Sony_D.jsv", "e401e30669938443c2f02522fd4d5aa1382931a0"},
  {"res/NLMQ1_JVC_C.264", "a86ec7a843e93f44aaee2619a7932c6c5c8d233f"},
  {"res/NLMQ2_JVC_C.264", "350ae86ef9ba09390d63a09b7f9ff54184109ca8"},
  {"res/NRF_MW_E.264", "5565843b3dcf39eb6b3a5580ce66379f220f7d1d"},
  {"res/QCIF_2P_I_allIPCM.264", "c7640f0704dcffb56ef3defea16e96bbd3aff822"},
  {"res/SVA_BA1_B.264", "4cb45a99ae44a0a98b174efd66245daa1fbaeb47"},
  {"res/SVA_BA2_D.264", "719fe839fa68b915b614fbbbae15edf492cc2133"},
  {"res/SVA_Base_B.264", "d45aabda059b2305e16894bd804d53d24e5b95e6"},
  {"res/SVA_CL1_E.264", "4fe09ab6cdc965ea10a20f1d6dd38aca954412bb"},
  {"res/SVA_FM1_E.264", "3ffab2fd4aae3e5f775ab36e05bf188ada825bc9"},
  {"res/SVA_NL1_B.264", "6d63f72a0c0d833b1db0ba438afff3b4180fb3e6"},
  {"res/SVA_NL2_E.264", "70453ef8097c94dd190d6d2d1d5cb83c67e66238"},
  {"res/SarVui.264", "719fe839fa68b915b614fbbbae15edf492cc2133"},
  {"res/Static.264", "914ba58c3215c939c45d5533e470fb1ec50024a7"},
  {"res/Zhling_1280x720.264", "a6c3e8edab5e67b563c98129017e5c590a38f228"},
  {"res/sps_subsetsps_bothVUI.264", "d65a34075c452196401340c554e83225c9454397"},
  {"res/test_cif_I_CABAC_PCM.264", "95fdf21470d3bbcf95505abb2164042063a79d98"},
  {"res/test_cif_I_CABAC_slice.264", "4260cc7a211895341092b0361bcfc3f13721ab44"},
  {"res/test_cif_P_CABAC_slice.264", "ddae50236add8aa64bdf8e67c942b58f5c24ae55"},
  {"res/test_qcif_cabac.264", "9186ba7836edbe58c513e1f2728952f891cb89ed"},
  {"res/test_scalinglist_jm.264", "12cb43d4e4066ea8523f6d7a668aa126038af4e6"},
  {"res/test_vd_1d.264", "4fd1c72e2ba500a6655b08fe5315168e577147b5"},
  {"res/test_vd_rc.264", "859ee4629bf55d9a24fa4b1b28c814020d899d05"},
  {"res/Cisco_Men_whisper_640x320_CABAC_Bframe_9.264", "d0f605c55816940eb7bd4fb66de0c16c3f0549db"},
  {"res/Cisco_Men_whisper_640x320_CAVLC_Bframe_9.264", "309b6affb28b73301acdc58f2c800d0bd81a8e27"},
  {"res/Cisco_Adobe_PDF_sample_a_1024x768_CAVLC_Bframe_9.264", "40cbd7ab6f62c2351598f551fb02aba8976a345f"},
  {"res/VID_1280x544_cabac_temporal_direct.264", "f6ba789f8d3fefdd8a051be26131349b04d557b4"},
  {"res/VID_1280x720_cabac_temporal_direct.264", "f29883e9888bc943b4d245238cd7fdfc92fc6804"},
  {"res/VID_1920x1080_cabac_temporal_direct.264", "11fb24c33258225d6fed66075c6696a54a4c1e22"},
  {"res/VID_1280x544_cavlc_temporal_direct.264", "9001ae769fd5f2baba4b42206a72c72f34744d69"},
  {"res/VID_1280x720_cavlc_temporal_direct.264", "099027a6857712811ed8e611275562e630a6aaae"},
  {"res/VID_1920x1080_cavlc_temporal_direct.264", "c32aafa080a9a63a4fbf6bee5fd5fc7873c3460d"},
};

INSTANTIATE_TEST_CASE_P (ThreadDecodeFile, ThreadDecoderOutputTest,
                         ::testing::ValuesIn (kFileParamArray));
