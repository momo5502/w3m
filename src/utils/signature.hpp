#pragma once
#include "nt.hpp"

namespace utils::hook
{
	class signature final
	{
	public:
		class signature_result
		{
		public:
			signature_result(std::vector<size_t>&& matches) : matches_(std::move(matches))
			{
				
			}

			uint8_t* get(const size_t index) const
			{
				if(index >= this->count())
				{
					throw std::runtime_error("Invalid index");
				}

				return reinterpret_cast<uint8_t*>(this->matches_[index]);
			}

			size_t count() const
			{
				return this->matches_.size();
			}

		private:
			std::vector<size_t> matches_;
		};
		
		explicit signature(const std::string& pattern, const nt::module module = {})
			: signature(pattern, module.get_ptr(), module.get_optional_header()->SizeOfImage)
		{
		}

		signature(const std::string& pattern, void* start, void* end)
			: signature(pattern, start, size_t(end) - size_t(start))
		{
		}

		signature(const std::string& pattern, void* start, const size_t length)
			: start_(static_cast<uint8_t*>(start)), length_(length)
		{
			this->load_pattern(pattern);
		}

		signature_result process();
		signature_result process_parallel();

	private:
		std::string mask_;
		std::basic_string<uint8_t> pattern_;
		
		uint8_t* start_;
		size_t length_;

		void load_pattern(const std::string& pattern);

		std::vector<size_t> process_range(void* start, size_t length);
	};
}
