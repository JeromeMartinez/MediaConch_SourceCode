# Creates source files without any compression in all native FFmpeg color spaces supported by FFmpeg FFV1 encoder
# then compress them to MKV/FFV1
# and compare the resulting decompressed frame with the source ("framemd5")
# Some color spaces are disabled as FFmpeg does not support them well during lossless compression.
# prerequisite: ffmpeg and diff in the path
# TODO: "bgr0" padding byte is 0xFF in raw, and 0x00 after FFmpeg FFV1 decompression, so framemd is different, should be fixed

function create_nut () {
echo $1
ffmpeg -hide_banner -y -f lavfi -i mandelbrot=s=4x4 -vf "format=$1" -t 1 -c rawvideo nut/$1.nut &>nul
}

function create_nut_yuv () {

for colorspace in gray yuv gbr; do
    if test "$colorspace" = "gbr"
    then
        bitdepth_list="9 10 12 14 16" #8-bit RGB is packed, not planar gbr, done elsewhere
        alpha_list="0"
    else
        bitdepth_list="8 9 10 12 16" #YUVxxxp14 is upscaled to 16-bit by FFV1 FFmpeg encoder, we don't test them
        alpha_list="0 a"
    fi
    for bitdepth in $bitdepth_list; do
        if test "$colorspace" = "yuv"
        then
            subsampling_list="444 440 422 420 411 410"
        else
            subsampling_list="n"
        fi
        for subsampling in $subsampling_list; do
            for alpha in $alpha_list; do
                if test "$colorspace" = "gray" && test "$alpha" = "a"
                then
                    colorspacemod="y"
                else
                    colorspacemod="$colorspace"
                fi
                if test "$alpha" = "a"
                then
                    alphamod="a"
                else
                    alphamod=""
                fi
                if test "$bitdepth" = "8"
                then
                    bitdepthmod=""
                else
                    bitdepthmod="$bitdepth""le"
                fi
                if test "$bitdepth" = "8" && test "$colorspace" = "gray" && test "$alpha" = "a"
                then
                    bitdepthmod="$bitdepth"
                fi
                if test "$subsampling" = "n"
                then
                    subsamplingmod=""
                else
                    subsamplingmod="$subsampling"
                fi
                if test "$colorspace" = "gray"
                then
                    planarmod=""
                else
                    planarmod="p"
                fi
                format="$colorspacemod""$alphamod""$subsamplingmod""$planarmod""$bitdepthmod"
                a=$(ffmpeg -hide_banner -pix_fmts | grep " $format"" ")
                if [ -n "$a" ]
                then
                    #ya16la is not supported by nut
                    #yuv440p10le is converted to rgb555 in nut format
                    #yuv440p12le is converted to rgb555 in nut format
                    if test "$format" != "ya16le" && test "$format" != "yuv440p10le" && test "$format" != "yuv440p12le" 
                    then
                        create_nut "$format" "$format"
                    fi
                fi
            done
        done
    done
done
}

# (TODO) file naming scheme:
# color space (rgb, yuv, xyz...) without details about color presence or alpha
# bX X bit per component
# ne (neutral, depends of architecture or no impact), le (Little Endian), be (Big Endian)
# nb neutral bits (8, 16...), lb for lower bits, hb higher bits / 
# planar or packed, neutral if 1 plane
# hX horizontal subsampling X
# vX vertical subsampling X
# currently using FFmpeg pix_fmt name

# Create Source files
echo "Creating source files for the following colors spaces:"
mkdir -p nut
create_nut "bgr0"
create_nut "bgra"
create_nut_yuv

# Create FFV1 files
echo "Creating corresponding FFV1 files..."
mkdir -p ffv1
cd nut
for file in *; do
    outputfile=${file//.nut/.mkv}
    ffmpeg -hide_banner -y -i "$file" -c ffv1 -level 3 -strict -2 "../ffv1/$outputfile" &>nul
done
cd ..

# Create framemd5 files - Source
echo "Creating FrameMD5 for source files..."
mkdir -p md5
cd nut
for file in *; do
    ffmpeg -hide_banner -y -i "$file" -f framemd5 "../md5/$file.md5" &>nul
done
cd ..

# Create framemd5 files - FFV1
echo "Creating FrameMD5 for FFV1 files..."
cd ffv1
for file in *; do
    ffmpeg -hide_banner -y -i "$file" -f framemd5 "../md5/$file.md5" &>nul
done
cd ..

# Do diff of framemd5 files
echo "Difference between source file and FFV1 file:"
cd nut
for file in *; do
    ffv1file=${file//.nut/.mkv}
    diff=$(diff "../md5/$file.md5" "../md5/$ffv1file.md5")
    if test ! -z "$diff"; then
        echo "$ffv1file"
    fi
done
cd ..
