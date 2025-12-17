module;

#include <imgui.h>
#include <imgui_stdlib.h>

export module xk.ImGuipp;
import std;

namespace xk::ImGuipp
{
	export class Platform
	{
	public:
		virtual void NewFrame() = 0;
		virtual ~Platform() = default;
	};

	export class Backend
	{

	public:
		virtual void NewFrame() = 0;
		virtual void Draw(ImDrawData* data, void* backendSpecific) = 0;
		virtual ~Backend() = default;
	};

	export class ImplContext
	{
	private:
		std::unique_ptr<Platform> platform;
		std::unique_ptr<Backend> backend;

	public:
		ImplContext(std::unique_ptr<Platform> platform, std::unique_ptr<Backend> backend) :
			platform{ std::move(platform) },
			backend{ std::move(backend) }
		{

		}

		template<class Func>
		void NewFrame(Func func, void* backendSpecific)
		{
			platform->NewFrame();
			backend->NewFrame();
			ImGui::NewFrame();

			try
			{
				func();
			}
			catch (...)
			{

			}

			ImGui::Render();
			backend->Draw(ImGui::GetDrawData(), backendSpecific);

			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}
	};


	struct ContextDeleter
	{
		void operator()(ImGuiContext* context) { ImGui::DestroyContext(context); }
	};

	export using UniqueContext = std::unique_ptr<ImGuiContext, ContextDeleter>;

	template<class Ty>
		requires std::integral<std::remove_cvref_t<Ty>> || std::floating_point<std::remove_cvref_t<Ty>>
	consteval ImGuiDataType_ ToDataType()
	{
		using type = std::remove_cvref_t<Ty>;
		if constexpr (std::same_as<type, bool>)
			return ImGuiDataType_Bool;
		if constexpr (std::same_as<type, signed char>)
			return ImGuiDataType_S8;
		if constexpr (std::same_as<type, char>)
			return ImGuiDataType_S8;
		if constexpr (std::same_as<type, unsigned char>)
			return ImGuiDataType_U8;
		if constexpr (std::same_as<type, std::int16_t>)
			return ImGuiDataType_S16;
		if constexpr (std::same_as<type, std::uint16_t>)
			return ImGuiDataType_U16;
		if constexpr (std::same_as<type, std::int32_t>)
			return ImGuiDataType_S32;
		if constexpr (std::same_as<type, std::uint32_t>)
			return ImGuiDataType_U32;
		if constexpr (std::same_as<type, std::int64_t>)
			return ImGuiDataType_S64;
		if constexpr (std::same_as<type, std::uint64_t>)
			return ImGuiDataType_U64;
		if constexpr (std::same_as<type, float>)
			return ImGuiDataType_Float;
		if constexpr (std::same_as<type, double>)
			return ImGuiDataType_Double;
	}

	export template<class Ty>
		struct OptionalChange
	{
	private:
		Ty value = {};
		bool changed = false;

	public:
		OptionalChange() = default;
		OptionalChange(Ty startingValue) :
			value{ startingValue }
		{

		}

		OptionalChange& operator=(const Ty& newValue)
		{
			changed = value != newValue;
			value = newValue;
			return *this;
		}

		OptionalChange& operator=(Ty&& newValue) noexcept
		{
			value = std::move(newValue);
			changed = true;
			return *this;
		}

		template<std::convertible_to<Ty> Ty2>
		OptionalChange& operator=(const OptionalChange<Ty2>& newValue)
		{
			changed = changed || newValue.Changed();
			value = newValue.Value() || value;
			return *this;
		}

		const bool& Changed() const noexcept { return changed; }
		const Ty& Value() const noexcept { return value; }

		template<class Ty>
		OptionalChange Apply(Ty function)
		{
			OptionalChange change = *this;
			change.changed = function(change.value).Changed() || change.changed;
			return change;
		}

		template<class Ty>
		OptionalChange Transform(Ty function)
		{
			OptionalChange change = *this;
			change.changed = function(change.value) || change.changed;
			return change;
		}

		operator std::tuple<Ty, bool>() const noexcept { return { value, changed }; }
		std::tuple<Ty, bool> Decompose() const noexcept { return { value, changed }; }
		operator bool() const noexcept { return Changed(); }
	};

	export bool Checkbox(std::string_view label, bool& enabled)
	{
		return ImGui::Checkbox(label.data(), &enabled);
	};

	export OptionalChange<bool> CheckboxCopy(std::string_view label, bool enabled)
	{
		return OptionalChange{ enabled }
			.Transform([label](bool& data) { return Checkbox(label, data); });
	};
	export template<class Ty>
		bool Combo(std::string_view label, Ty& currentValue, std::span<const Ty> values, std::span<const std::string_view> stringValues)
	{
		std::size_t currentIndex = std::ranges::find(values, currentValue) - values.begin();
		bool changed = false;
		if (ImGui::BeginCombo(label.data(), (currentIndex >= 0 && currentIndex < stringValues.size()) ? stringValues[currentIndex].data() : ""))
		{
			for (std::size_t i = 0; i < values.size(); i++)
			{
				const bool is_selected = (currentValue == values[i]);
				if (ImGui::Selectable(stringValues[i].data(), is_selected))
				{
					currentValue = values[i];
					changed = true;
				}
				// Set the initial focus when opening the ComboCopy (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		return changed;
	};

	export template<class Ty>
	bool Combo(std::string_view label, const Ty& currentValue, std::span<const Ty> values, std::span<const std::string_view> stringValues)
	{
		std::size_t currentIndex = std::ranges::find(values, currentValue) - values.begin();
		bool changed = false;
		if (ImGui::BeginCombo(label.data(), (currentIndex >= 0 && currentIndex < stringValues.size()) ? stringValues[currentIndex].data() : ""))
		{
			ImGui::BeginDisabled();
			for (std::size_t i = 0; i < values.size(); i++)
			{
				const bool is_selected = (currentValue == values[i]);

				ImGui::Selectable(stringValues[i].data(), is_selected);
				// Set the initial focus when opening the ComboCopy (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}

		return changed;
	};

	export template<class Ty>
	OptionalChange<Ty> ComboCopy(std::string_view label, Ty currentValue, std::span<const Ty> values, std::span<const std::string_view> stringValues)
	{
		return OptionalChange{ currentValue }
			.Transform([values, stringValues](Ty& data)
		{
			return Combo<Ty>(label.data(), data, stringValues.data(), stringValues.size());
		});
	};

	//export bool InputField(std::string_view label, std::string_view data, ImGuiInputTextFlags flags = 0)
	//{
	//	return ImGui::InputText(label.data(), data.data(), (int)data.size() + 1, flags);
	//}

	export bool InputField(std::string_view label, std::string& data, ImGuiInputTextFlags flags = 0)
	{
		return ImGui::InputText(label.data(), &data, flags);
	}

	//export bool InputField(std::string_view label, const std::string& data, ImGuiInputTextFlags flags = 0)
	//{
	//	return InputField(label, std::string_view{ data }, flags);
	//}

	//export bool InputFieldMultiline(std::string_view label, std::string_view data, ImGuiInputTextFlags flags = 0)
	//{
	//	return ImGui::InputTextMultiline(label.data(), data.data(), (int)data.size() + 1, ImVec2(0, 0), flags);
	//}

	export bool InputFieldMultiline(std::string_view label, std::string& data, ImGuiInputTextFlags flags = 0)
	{
		return ImGui::InputTextMultiline(label.data(), &data, {}, flags);
	}

	//export bool InputFieldMultiline(std::string_view label, const std::string& data, ImGuiInputTextFlags flags = 0)
	//{
	//	return InputFieldMultiline(label, std::string_view{ data }, flags);
	//}

	//export OptionalChange<std::string_view> InputFieldCopy(std::string_view label, std::string_view data, ImGuiInputTextFlags flags = 0)
	//{
	//	return OptionalChange{ data }
	//	.Transform([label, flags](std::string_view& data) { return InputField(label.data(), data, flags); });
	//}

	export OptionalChange<std::string> InputFieldCopy(std::string_view label, std::string data, ImGuiInputTextFlags flags = 0)
	{
		return OptionalChange{ data }
		.Transform([label, flags](std::string& data) { return InputField(label.data(), data, flags); });
	}

	export template<class Ty>
		requires std::integral<std::remove_cvref_t<Ty>> || std::floating_point<std::remove_cvref_t<Ty>>
	bool InputField(std::string_view label, Ty & data, ImGuiInputTextFlags flags = 0)
	{
		return ImGui::InputScalar(label.data(), ToDataType<Ty>(), &data, nullptr, nullptr, nullptr, flags);
	}

	export template<class Ty>
		requires std::integral<std::remove_cvref_t<Ty>> || std::floating_point<std::remove_cvref_t<Ty>>
	bool InputField(std::string_view label, const Ty & data, ImGuiInputTextFlags flags = 0)
	{
		return ImGui::InputScalar(label.data(), ToDataType<Ty>(), &data, nullptr, flags);
	}

	export template<class Ty>
		requires std::integral<std::remove_cvref_t<Ty>> || std::floating_point<std::remove_cvref_t<Ty>>
	OptionalChange<Ty> InputFieldCopy(std::string_view label, Ty data, ImGuiInputTextFlags flags = 0)
	{
		return OptionalChange{ data }
		.Transform([label, flags](Ty& data) { return InputField(label, data, flags); });
	}

	export template<class Ty>
	struct Reflect;

	export template<class Ty>
	bool Field(std::string_view label, Ty& data)
	{
		return Reflect<Ty>::Draw(data);
	}

	export template<class Ty>
		requires std::integral<Ty> || std::floating_point<Ty>
	bool Field(std::string_view label, Ty& data)
	{
		return InputField(label, data);
	}
}