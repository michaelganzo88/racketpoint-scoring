#
# RacketPoint padel scoring engine — Flutter macOS FFI plugin.
#
Pod::Spec.new do |s|
  s.name             = 'racketpoint_scoring'
  s.version          = '0.2.0'
  s.summary          = 'RacketPoint padel scoring engine (C99) — Flutter FFI'
  s.homepage         = 'https://github.com/michaelganzo88/racketpoint-scoring'
  s.license          = { :type => 'MIT' }
  s.author           = { 'RacketPoint' => 'info@racketpoint.app' }
  s.source           = { :path => '.' }

  # C sources (relative to this podspec, which lives in macos/)
  s.source_files     = '../padel_scoring.{h,c}',
                       '../rp_ffi.{h,c}',
                       '../scoring.h'

  s.dependency 'FlutterMacOS'
  s.platform         = :osx, '10.14'

  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'OTHER_CFLAGS'   => '-DDART_SHARED_LIB',
  }
end
