﻿#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include "ragii/image/Bitmap.h"
#include "ragii/image/BitmapConverter.h"
#include "ragii/text/text.h"


using namespace std;
using namespace ragii::image;


int convert(vector<pair<string_view, string_view>>& opts);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		return EXIT_FAILURE;
	}

	vector<pair<string_view, string_view>> opts;

	for (int i = 1; i < argc; )
	{
		auto opt = string_view(argv[i]);
		if (opt[0] == '-')
		{
			opts.emplace_back(argv[i], argv[i + 1]);
			i += 2;
		}
		else
		{
			opts.emplace_back(argv[i], "");
			i++;
		}
	}

	for (auto i : opts)
	{
		cout << i.first.data() << ": " << i.second.data() << endl;
	}

	if (opts.empty())
	{
		cout << "コマンドを指定してください。 convert, etc." << endl;
		return EXIT_FAILURE;
	}

	auto command = opts[0].first;
	opts.erase(opts.begin());

	if (command == "convert")
	{
		return convert(opts);
	}

	return EXIT_SUCCESS;
}


int convert(vector<pair<string_view, string_view>>& opts)
{
	if (opts.empty())
	{
		cout << "変換方法を指定してください。 negative, grayscale, etc." << endl;
		return EXIT_FAILURE;
	}

	auto in_file = find_if(opts.begin(), opts.end(), [&](auto i) { return i.first == "-i"; });
	if (in_file == opts.end())
	{
		cout << "入力ファイル名を指定してください。" << endl;
		return EXIT_FAILURE;
	}

	auto out_file = find_if(opts.begin(), opts.end(), [&](auto i) { return i.first == "-o"; });
	if (out_file == opts.end())
	{
		cout << "出力ファイル名を指定してください。" << endl;
		return EXIT_FAILURE;
	}

	if (!ragii::text::ends_with(in_file->second.data(), ".bmp"))
	{
		cout << ".bmp 以外は非対応です。" << endl;
		return EXIT_FAILURE;
	}

	auto bmp = Bitmap::loadFromFile(in_file->second.data());

	if (!bmp)
	{
		cout << "ファイルのロードに失敗しました。" << endl;
		return EXIT_FAILURE;
	}

	auto filter = opts[0].first;

	if (filter == "negative")
	{
		BitmapConverter::applyFilter(bmp.get(), FilterType::Negative);
	}
	else if (filter == "binary")
	{
		BitmapConverter::applyFilter(bmp.get(), FilterType::Binary);
	}
	else if (filter == "grayscale")
	{
		BitmapConverter::applyFilter(bmp.get(), FilterType::Grayscale);
	}
	else if (filter == "laplacian")
	{
		BitmapConverter::applyFilter(bmp.get(), FilterType::Grayscale);
		BitmapConverter::applyFilter(bmp.get(), FilterType::Laplacian);
	}

	bmp->save(out_file->second.data());

	return EXIT_SUCCESS;
}
