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
    std::string p_hashStr (p.hashStr);
    std::stringstream ss (p_hashStr);
    std::string buf[4];
    const char* hashStr[4];
    int i = 0;
    while (i < 4 && ss >> buf[i]) {
      hashStr[i] = buf[i].c_str();
      ++i;
    }
    CompareHashAnyOf (digest, hashStr, i);
  }
}
static const FileParam kFileParamArray[] = {
  {"res/Adobe_PDF_sample_a_1024x768_50Frms.264", "041434a5819d1d903d49c0eda884b345e9f83596 9aa9a4d9598eb3e1093311826844f37c43e4c521"},
  {"res/BA1_FT_C.264", "072ccfd92528f09ae8888cb5e023af511e1010a1 3e7a012a01904cdc78c63ae20235665441b4e0a7"},
  {"res/BA1_Sony_D.jsv", "37c9a951a0348d6abe1880b59e2b5a4d7d18c94c"},
  {"res/BAMQ1_JVC_C.264", "6720462624f632f5475716ef32a7bbd12b3b428a 477b1e45e30661a138ff0b43c1ed3e00ded13d9c"},
  {"res/BAMQ2_JVC_C.264", "5f0fbb0dab7961e782224f6887c83d4866fc1af8 e3dfdc770fa5fee8b92f896a92214886c109a688"},
  {"res/BA_MW_D.264", "ace02cdce720bdb0698b40dc749a0e61fe0f590b"},
  {"res/BANM_MW_D.264", "c51f1d2fa63dba4f5787f1b726c056d1c01d6ab9"},
  {"res/BASQP1_Sony_C.jsv", "2e10e98fc54f92cb5e72513bf417c4e4df333361 d5e1f122e8bf8d58bc6775d69b837db0d9ea3454"},
  {"res/CI1_FT_B.264", "721e555a33cfff81b6034a127334c5891776373c be9d94803e6e7b43cf9283739024077e791199e5"},
  {"res/CI_MW_D.264", "49a8916edd3e571efad328f2784fbe6aec5570d7"},
  {"res/CVFC1_Sony_C.jsv", "824ae1dd1b6dc9f2acf1b014bcc0886ad16fa090 f577bd279e204d48317becfd5249405a7eb90312"},
  {"res/CVPCMNL1_SVA_C.264", "c2b0d964de727c64b9fccb58f63b567c82bda95a"},
  {"res/LS_SVA_D.264", "72118f4d1674cf14e58bed7e67cb3aeed3df62b9 e020a1c6668501887bb55e00741ebfdbc91d400d"}, //DPB buffer is too small
  {"res/MIDR_MW_D.264", "aeded2be7b97484cbf25f367ec34208f2220a8ab"},
  //{"res/MPS_MW_A.264", "b0fce28218e678d89f464810f88b143ada49dd06"}, //hanging
  //{"res/MR1_BT_A.h264", "eebd1d7cdb67df5b8688b1ce18f6acae129b32e6 16df32765c45a0deef8efcfc98cb150fdbd5f12c 9f2b87ef4d3d051451107079e4821d2c41754f87"}, //three hash values temp disabled
  {"res/MR1_MW_A.264", "14d8ddb12ed711444039329db29c496b079680ba"},
  {"res/MR2_MW_A.264", "6d332a653fe3b923eb3af8f3695d46ce2a1d4b2c"},
  //{"res/MR2_TANDBERG_E.264", "74d618bc7d9d41998edf4c85d51aa06111db6609"}, //DPB buffer is too small
  {"res/NL1_Sony_D.jsv", "e401e30669938443c2f02522fd4d5aa1382931a0"},
  {"res/NLMQ1_JVC_C.264", "f3265c6ddf8db1b2bf604d8a2954f75532e28cda a86ec7a843e93f44aaee2619a7932c6c5c8d233f"},
  {"res/NLMQ2_JVC_C.264", "350ae86ef9ba09390d63a09b7f9ff54184109ca8 95e6e4426b75f38a6744f3d04cfc62a2c0489354"},
  {"res/NRF_MW_E.264", "866f267afd2ed1595bcb90de0f539e929c169aa4 db2d135cef07db8247ef858daf870d07955b912a"},
  {"res/QCIF_2P_I_allIPCM.264", "9879ce127d3263cfbaf5211ab6657dbf0ccabea8"},
  { "res/SVA_BA1_B.264", "4cb45a99ae44a0a98b174efd66245daa1fbaeb47 e9127875b268f9e7da4c495799b9972b8e72cf7b"},
  {"res/SVA_BA2_D.264", "ac9e960015b96f83279840802f6637c61ee1c5b8 719fe839fa68b915b614fbbbae15edf492cc2133"},
  {"res/SVA_Base_B.264", "e6010d1b47aa796c1f5295b2563ed696aa9c37ab d8f923b278e6e9cbf51b495b29c2debe53526518"},
  {"res/SVA_CL1_E.264", "4fe09ab6cdc965ea10a20f1d6dd38aca954412bb"},
  {"res/SVA_FM1_E.264", "1a114fbd096f637acd0c3fb8f35bdfa3bc275199 dc22699d39caf9eb1d32ecd4966869578d24cd86"},
  {"res/SVA_NL1_B.264", "6d63f72a0c0d833b1db0ba438afff3b4180fb3e6"},
  {"res/SVA_NL2_E.264", "70453ef8097c94dd190d6d2d1d5cb83c67e66238"},
  //{"res/SarVui.264", "1843d19d8e13588ef5de2d647804ae141e55cf72 719fe839fa68b915b614fbbbae15edf492cc2133"}, //same as "res/SVA_BA1_B.264"
  {"res/Static.264", "d865faee7df56a8f532b7baeacb814483b8be148 52af285a888b8c9e04dc9f38fd61105e805ada3a 1b6313262bff9c329aaf7dd3582525bd609c3974"},
  {"res/Zhling_1280x720.264", "10f9c803e80b51786f7833255afc3ef75c5c1339"},
  //{"res/sps_subsetsps_bothVUI.264", "d65a34075c452196401340c554e83225c9454397"}, //hanging
  {"res/test_cif_I_CABAC_PCM.264", "dfe2f87ac76bdb58e227267907a2eeccf04715ad 95fdf21470d3bbcf95505abb2164042063a79d98"},
  {"res/test_cif_I_CABAC_slice.264", "a7154eb1d0909eb9fd1e4e89f5d6271e5201814b 106da52c2c6d30255b6ac0aa0b4a881a06ebb762"},
  {"res/test_cif_P_CABAC_slice.264", "b08bcf1056458ae113d0a55f35e6b00eb2bd7811 fdcb320d8a5721f62c960c96342494982eda5e42 7d755833be8c017560e04446b01458d6109619a2"}, //3 hashes
  {"res/test_qcif_cabac.264", "c79e9a32e4d9e38a1bd12079da19dcb0d2efe539"},
  //{"res/test_scalinglist_jm.264", "b36efd05c8b17faa23f1c071b92aa5d55a5a826f"}, //hanging
  {"res/test_vd_1d.264", "15d8beaf991f9e5d56a854cdafc0a7abdd5bec69"},
  {"res/test_vd_rc.264", "cd6ef57fc884e5ecd9867591b01e35e3f091b8d0"},
  {"res/Cisco_Men_whisper_640x320_CABAC_Bframe_9.264", "5d3d08fb47ac8c6e379c1572aed517522d883920 f5593d374e8f68b1c882d407d961d80cf10ba737"},
  {"res/Cisco_Men_whisper_640x320_CAVLC_Bframe_9.264", "89742b454cac4843e0bf18a3df9b46f21155b48a 67eec8abb0b22ff0f00d06c769b8a2e44cec33cf"},
  {"res/Cisco_Adobe_PDF_sample_a_1024x768_CAVLC_Bframe_9.264", "5fce0b92c5f2a1636ea06ae48ea208908fd01416"},
  {"res/VID_1280x544_cabac_temporal_direct.264", "ae5f21eff917d09d5a1ba2ad2075edd92eb6b61c"},
  {"res/VID_1280x720_cabac_temporal_direct.264", "2597181429a48740a143053a5b027dcbe4173f4e"},
  {"res/VID_1920x1080_cabac_temporal_direct.264", "e8e686d4cd07cc39b4da5c5050fd500c81a2c715 1a31a34902e9c26ce8cee50744d86e6e429f0ff4"},
  {"res/VID_1280x544_cavlc_temporal_direct.264", "d9b31a2586ee156fe697de5934afb5a769f79494"},
  {"res/VID_1280x720_cavlc_temporal_direct.264", "888c31cef73eb6804e2469fa77e51636c915ff82"},
  {"res/VID_1920x1080_cavlc_temporal_direct.264", "b38f9817d8bd7b2d4ba483d9e71bb7d279610325 9085d7bbaf45d9414582727d30812c8fbedee5e5"},
};

INSTANTIATE_TEST_CASE_P (ThreadDecodeFile, ThreadDecoderOutputTest,
                         ::testing::ValuesIn (kFileParamArray));
