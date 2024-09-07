#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace cs_std
{
	typedef uint8_t byte;

	class file
	{
	protected:
		std::filesystem::path file_path;
		std::fstream stream;
		file(const std::filesystem::path& filePath) : file_path(filePath) {}
	public:
		file() = delete;
		virtual ~file() { this->close(); }
		void close()
		{
			if (!this->is_open()) return;
			this->stream.close();
		}
		bool exists() const { return std::filesystem::exists(this->file_path); }
		bool is_open() const { return this->stream.is_open(); }
		void create()
		{
			std::fstream tempStream;
			tempStream.open(this->file_path, std::ios_base::out);
			if (!tempStream.is_open()) throw std::runtime_error("Failed to create the file.");
			tempStream.close();
		}
		void remove()
		{
			this->close();
			if (!std::filesystem::remove(this->file_path)) throw std::runtime_error("Failed to remove the file.");
		}
		void rename(const std::string& name)
		{
			std::filesystem::path parentDir = file_path.parent_path();
			std::filesystem::path newPath = parentDir / name;
			this->close();
			try
			{
				std::filesystem::rename(file_path, newPath);
				this->file_path = newPath;
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				throw std::runtime_error("Failed to rename the file: " + std::string(e.what()));
			}
		}
		void clear()
		{
			std::fstream tempStream(this->file_path, std::ios::out | std::ios::trunc);
			if (!tempStream.is_open()) throw std::runtime_error("Failed to clear the file.");
			tempStream.close();
		}
		size_t size() const
		{
			try {
				return std::filesystem::file_size(this->file_path);
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				throw std::runtime_error("Failed to get the file size: " + std::string(e.what()));
			}
		}
		std::string name() const { return this->file_path.filename().string(); }
		std::string extension() const { return this->file_path.extension().string(); }
		std::filesystem::path path() const { return this->file_path; }
	};
	class binary_file : public file
	{
	public:
		binary_file(const std::filesystem::path& filePath) : file(filePath) {}
		binary_file& open()
		{
			this->stream.open(this->file_path, std::ios::in | std::ios::out | std::ios::binary);
			if (!this->stream.is_open()) throw std::runtime_error("Failed to open the binary file.");
			return *this;
		}
		std::vector<byte> read(size_t start, size_t count = 1)
		{
			if (!this->is_open()) throw std::runtime_error("File is not open.");
			this->stream.seekg(start, std::ios::beg);
			if (!this->stream.good()) throw std::runtime_error("Failed to seek to the specified position in the binary file.");
			std::vector<byte> buffer(count);
			this->stream.read(reinterpret_cast<char*>(buffer.data()), count);
			if (this->stream.fail() && !this->stream.eof()) throw std::runtime_error("Error occurred while reading from the binary file.");
			return buffer;
		}
		std::vector<byte> read()
		{
			return this->read(0, this->size());
		}
		std::vector<byte> read_if_exists()
		{
			if (this->exists()) return this->read();
			return std::vector<byte>();
		}
		binary_file& append(const std::vector<byte>& data)
		{
			if (!this->is_open()) throw std::runtime_error("File is not open.");
			this->stream.seekp(0, std::ios::end);
			if (!this->stream.good()) throw std::runtime_error("Failed to seek to the end of the binary file.");
			this->stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			if (this->stream.fail()) throw std::runtime_error("Error occurred while writing to the binary file.");
			return *this;
		}
	};

	class text_file : public file
	{
	public:
		text_file(const std::filesystem::path& filePath) : file(filePath) {}
		text_file& open()
		{
			this->stream.open(this->file_path, std::ios::in | std::ios::out);
			if (!this->stream.is_open()) throw std::runtime_error("Failed to open the text file.");
			return *this;
		}
		std::string read(size_t start, size_t count = 1)
		{
			if (!this->is_open()) throw std::runtime_error("File is not open.");
			this->stream.seekg(start, std::ios::beg);
			if (!this->stream.good()) throw std::runtime_error("Failed to seek to the specified position in the text file.");
			std::string buffer;
			buffer.resize(count);
			this->stream.read(&buffer[0], count);

			if (this->stream.fail() && !this->stream.eof()) throw std::runtime_error("Error occurred while reading from the text file.");
			return buffer;
		}
		std::string read()
		{
			return this->read(0, this->size());
		}
		std::string read_if_exists()
		{
			if (this->exists()) return this->read();
			return std::string();
		}
		text_file& append(const std::string& data)
		{
			if (!this->is_open()) throw std::runtime_error("File is not open.");
			this->stream.seekp(0, std::ios::end);
			if (!this->stream.good()) throw std::runtime_error("Failed to seek to the end of the text file.");
			this->stream.write(data.c_str(), data.size());
			if (this->stream.fail()) throw std::runtime_error("Error occurred while writing to the text file.");

			return *this;
		}
		text_file& append_line(const std::string& data)
		{
			this->append(data);
			this->append("\n");
			return *this;
		}
	};
}
